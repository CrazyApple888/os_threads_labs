fn do_job() -> std::thread::Result<()> {
    std::thread::spawn(move || {
        for _i in 0..10 {
            println!("Hello from new thread!");
        }
    }).join()
}

fn main() {
    match do_job()
    {
        Ok(_) => {
            for _i in 0..10 {
                println!("Hello from old thread!");
            }
        }
        Err(e) => std::panic::resume_unwind(e),
    }
}