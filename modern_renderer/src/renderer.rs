use winit;
use winit::platform::windows::WindowExtWindows;
use rusty_d3d12::*;
mod render_utils;

#[no_mangle]
pub static D3D12SDKVersion: u32 = 600;

#[no_mangle]
pub static D3D12SDKPath: &[u8; 9] = b".\\D3D12\\\0";

pub const FRAMES_IN_FLIGHT: u32 = 3;

pub struct Renderer {
    device: rusty_d3d12::Device,
    command_queue: rusty_d3d12::CommandQueue,
    fence: rusty_d3d12::Fence,
    fence_event: rusty_d3d12::Win32Event,
    fence_values: [u64; FRAMES_IN_FLIGHT as usize],
    swapchain: rusty_d3d12::Swapchain,
    frame_index: usize,
    frame_count: u32,
    viewport_desc: rusty_d3d12::Viewport,
    scissor_desc: rusty_d3d12::Rect,
    render_targets: Vec<rusty_d3d12::Resource>,
    rtv_heap: rusty_d3d12::DescriptorHeap,
    rtv_descriptor_handle_size: rusty_d3d12::ByteCount,
    dsv_heap: rusty_d3d12::DescriptorHeap,
    command_allocators: Vec<rusty_d3d12::CommandAllocator>,
    command_list: rusty_d3d12::CommandList,
    depth_stencil: Option<rusty_d3d12::Resource>,
    path_tracing_pso : rusty_d3d12::PipelineState,
    root_signature : rusty_d3d12::RootSignature,
}

impl Renderer {

    pub fn new(window: &winit::window::Window) -> Self {

        // // Enable validation layer
        // let debug_layer = Debug::new().expect("Cannot create debug layer");
        // debug_layer.enable_debug_layer();
        // debug_layer.enable_gpu_based_validation();
        // debug_layer.enable_object_auto_name();

        let factory_flags = rusty_d3d12::CreateFactoryFlags::None;
        let factory = Factory::new(factory_flags).expect("Cannot create factory");

        let hw_adapter = factory
            .enum_adapters()
            .expect("Cannot enumerate adapters")
            .remove(0);
        let device = Device::new(&hw_adapter).expect("Cannot create device");

        let command_queue = device
            .create_command_queue(&CommandQueueDesc::default())
            .expect("Cannot create command queue");

        let fence = device
            .create_fence(0, FenceFlags::None)
            .expect("Cannot create fence");

        let fence_event = Win32Event::default();
        let fence_values = [0; FRAMES_IN_FLIGHT as usize];
        let frame_index = 0;

        let window_size = window.inner_size();

        let mut swapchain_desc = SwapChainDesc::default();

        swapchain_desc.set_width(window_size.width)
        .set_height(window_size.height)
        .set_buffer_count(u32::from(FRAMES_IN_FLIGHT));

        let swapchain = factory
            .create_swapchain(&command_queue, window.hwnd() as *mut HWND__, &swapchain_desc)
            .expect("Cannot create swapchain");
        factory
            .make_window_association(window.hwnd(), MakeWindowAssociationFlags::NoAltEnter)
            .expect("Cannot make window association");

        let mut viewport_desc = Viewport::default();

            viewport_desc.set_width(window_size.width as f32)
            .set_height(window_size.height as f32);

        let mut scissor_desc = Rect::default();

            scissor_desc.set_right(window_size.width as i32)
            .set_bottom(window_size.height as i32);

        let rtv_descriptor_handle_size = device
            .get_descriptor_handle_increment_size(DescriptorHeapType::Rtv);

        let rtv_heap = device
            .create_descriptor_heap(
                &DescriptorHeapDesc::default()
                    .set_heap_type(DescriptorHeapType::Rtv)
                    .set_num_descriptors(u32::from(FRAMES_IN_FLIGHT)),
            )
            .expect("Cannot create RTV heap");
        rtv_heap
            .set_name("RTV heap")
            .expect("Cannot set RTV heap name");

        let dsv_heap = device
            .create_descriptor_heap(
                &DescriptorHeapDesc::default()
                    .set_heap_type(DescriptorHeapType::Dsv)
                    .set_num_descriptors(1),
            )
            .expect("Cannot create RTV heap");
        dsv_heap
            .set_name("DSV heap")
            .expect("Cannot set DSV heap name");

        let mut rtv_handle = rtv_heap.get_cpu_descriptor_handle_for_heap_start();

        let mut render_targets = vec![];
        for frame_idx in 0..FRAMES_IN_FLIGHT {
            let render_target = swapchain
                .get_buffer(u32::from(frame_idx))
                .expect("cannot get buffer from swapchain");

            device.create_render_target_view(&render_target, rtv_handle);
            render_targets.push(render_target);

            rtv_handle = rtv_handle.advance(1, rtv_descriptor_handle_size);
        }

        let mut command_allocators = vec![];
        for _ in 0..FRAMES_IN_FLIGHT {
            command_allocators.push(
                device
                    .create_command_allocator(CommandListType::Direct)
                    .expect("Cannot create command allocator"),
            );
        }

        let command_list = device
            .create_command_list(
                CommandListType::Direct,
                &command_allocators[0],
                None,
            )
            .expect("Cannot create command list");
        command_list.close().expect("Cannot close command list");

        let root_signature = render_utils::setup_root_signature(&device);

        let path_tracing_compute = render_utils::compile_shader(
            "PathTracing",
            r"#
            RWTexture2D<float4> output : register(u0);
            
            [numthreads(8, 8, 1)]
            void CSMain(uint3 dispatchThreadId : SV_DispatchThreadID)
            {
                output[dispatchThreadId.xy] = float4(dispatchThreadId.x & 1, dispatchThreadId.y & 2, dispatchThreadId.x & 4, 1);
            }
            #",
            "CSMain",
            "cs_6_0"
        );

        let path_tracing_pso = render_utils::create_compute_pso(
            &root_signature,
            path_tracing_compute,
            &device,
        );

        let renderer = Self {
            device,
            command_queue,
            fence,
            fence_event,
            fence_values,
            swapchain,
            frame_index,
            frame_count: 0,
            viewport_desc: viewport_desc,
            scissor_desc: scissor_desc,
            render_targets,
            rtv_heap,
            rtv_descriptor_handle_size,
            dsv_heap,
            command_allocators,
            command_list,
            depth_stencil: None,
            path_tracing_pso,
            root_signature
        };

        renderer
    }

    pub fn execute(&mut self) {
        let last_fence_value = self.fence_values[self.frame_index];
        let fence_completed_value = self.fence.get_completed_value();

        if fence_completed_value < last_fence_value {
            self.fence
                .set_event_on_completion(last_fence_value, &self.fence_event)
                .expect("Cannot set event on fence");

            self.fence_event.wait(None);
        }

        self.command_allocators[self.frame_index]
        .reset()
        .expect("Cannot reset command allocator");
    
        // Clear command buffer to start recording the next frame
        self.command_list
            .reset(&self.command_allocators[self.frame_index], None)
            .expect("Cannot reset command list");

        self.command_list.set_viewports(&[self.viewport_desc]);
        self.command_list.set_scissor_rects(&[self.scissor_desc]);

        let mut current_render_target = &self.render_targets[self.frame_index as usize];
        let rt_desc = current_render_target.get_desc();

        // tell the driver that the current render target will be written to
        self.command_list
            .resource_barrier(&[ResourceBarrier::new_transition(
                &ResourceTransitionBarrier::default()
                    .set_resource(
                        current_render_target,
                    )
                    .set_state_before(ResourceStates::Common)
                    .set_state_after(ResourceStates::RenderTarget),
            )]);

        let rtv_handle = self
            .rtv_heap
            .get_cpu_descriptor_handle_for_heap_start()
            .advance(
                self.swapchain.get_current_back_buffer_index(),
                self.rtv_descriptor_handle_size,
            );

        // Clear render target 
        self.command_list.set_render_targets(
            &mut [rtv_handle],
            false,
            Some(self.dsv_heap.get_cpu_descriptor_handle_for_heap_start()),
        );

        let clear_color: [f32; 4] = [0.0, 0.0, 0.0, 1.0];
        self.command_list.clear_render_target_view(
            rtv_handle,
            clear_color,
            &[],
        );

        self.command_list.set_compute_root_signature(&self.root_signature);
        self.command_list.set_pipeline_state(&self.path_tracing_pso);
        // self.command_list.set_compute_root_descriptor_table(0, self.dsv_heap.get_gpu_descriptor_handle_for_heap_start());
        // self.command_list.set_compute_root_descriptor_table(0, self.rtv_heap.get_gpu_descriptor_handle_for_heap_start());
    
        self.command_list.dispatch(rt_desc.width() as u32 / 8, rt_desc.height() / 8, 1);

        //self.command_list.dispatch_mesh(self.meshlet_count, 1, 1);

        self.command_list
            .resource_barrier(&[ResourceBarrier::new_transition(
                &ResourceTransitionBarrier::default()
                    .set_resource(
                        &self.render_targets[self.frame_index as usize],
                    )
                    .set_state_before(ResourceStates::RenderTarget)
                    .set_state_after(ResourceStates::Common),
            )]);

        self.command_list
            .close()
            .expect("Cannot close command list");

        self.command_queue.execute_command_lists(std::slice::from_ref(&self.command_list));

        self.swapchain
            .present(1, PresentFlags::None)
            .expect("Cannot present");

        self.fence_values[self.frame_index] = last_fence_value + 1;

        self.command_queue
            .signal(&self.fence, self.fence_values[self.frame_index])
            .expect("Cannot signal fence");

        self.frame_index = (self.frame_index + 1) % FRAMES_IN_FLIGHT as usize;

        self.fence_values[self.frame_index as usize] = last_fence_value + 1;

        self.frame_count += 1;

    }
}
