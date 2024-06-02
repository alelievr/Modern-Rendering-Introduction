mod renderer;
use renderer::*;

use winit::{
    event::{Event, WindowEvent},
    event_loop::{ControlFlow, EventLoop},
    window::WindowBuilder,
};

const WINDOW_WIDTH: u32 = 640;
const WINDOW_HEIGHT: u32 = 480;


fn main() {
    let event_loop = EventLoop::new();
    let window = WindowBuilder::new()
        .with_title("Modern Renderer")
        .build(&event_loop)
        .expect("Cannot create window");
    window.set_inner_size(winit::dpi::LogicalSize::new(
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
    ));

    let mut renderer = Renderer::new(&window);

    event_loop.run(move |event, _, control_flow| {
        *control_flow = ControlFlow::Poll;
        match event {
            Event::WindowEvent {
                event: WindowEvent::CloseRequested,
                ..
            } => *control_flow = ControlFlow::Exit,
            Event::MainEventsCleared => {
                window.request_redraw();
            }
            Event::RedrawRequested(_) => {
                renderer.execute();
            }
            _ => (),
        }
    });
}
