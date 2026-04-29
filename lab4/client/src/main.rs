use std::io::{Read, Write};
use std::net::TcpStream;
use std::{thread, time};

const SERVER_ADDR: &str = "127.0.0.1:8080";
const POLL_INTERVAL_MS: u64 = 100;

#[repr(u8)]
enum CommandType {
    ConfigAndData = 1,
    StartComputation = 2,
    RequestStatus = 3,
    StatusResponse = 4,
}

#[repr(u8)]
enum ClientStatus {
    Running = 1,
    Done = 2,
    Error = 3,
}

fn main() -> std::io::Result<()> {
    let mut stream = TcpStream::connect(SERVER_ADDR)?;
    println!("Connected to server");

    let num_threads: u32 = 4;
    let n: u32 = 5;
    let matrix: Vec<u32> = (0..n * n).map(|i| (i as u32 % 100) + 1).collect();

    println!("Original matrix:");
    print_matrix(&matrix, n);

    let mut payload = Vec::new();
    payload.push(CommandType::ConfigAndData as u8);
    payload.extend_from_slice(&num_threads.to_be_bytes());
    payload.extend_from_slice(&n.to_be_bytes());
    for &val in &matrix {
        payload.extend_from_slice(&val.to_be_bytes());
    }

    send_msg(&mut stream, &payload)?;

    let ack = recv_msg(&mut stream)?;
    if ack.len() != 1 || ack[0] != CommandType::ConfigAndData as u8 {
        println!("Failed to receive ACK for config");
        return Ok(());
    }
    println!("Config sent, ACK received");

    let payload2 = vec![CommandType::StartComputation as u8];
    send_msg(&mut stream, &payload2)?;

    let ack2 = recv_msg(&mut stream)?;
    if ack2.len() != 1 || ack2[0] != CommandType::StartComputation as u8 {
        println!("Failed to receive ACK for start");
        return Ok(());
    }
    println!("Start command sent, ACK received");

    loop {
        let payload3 = vec![CommandType::RequestStatus as u8];
        send_msg(&mut stream, &payload3)?;

        let resp = recv_msg(&mut stream)?;
        if resp.len() >= 2 && resp[0] == CommandType::StatusResponse as u8 {
            let status = resp[1];
            if status == ClientStatus::Done as u8 {
                if resp.len() >= 6 {
                    let mut n_bytes = [0u8; 4];
                    n_bytes.copy_from_slice(&resp[2..6]);
                    let recv_n = u32::from_be_bytes(n_bytes);

                    let mut result_matrix = Vec::new();
                    let num_elements = recv_n * recv_n;
                    for i in 0..num_elements as usize {
                        let mut val_bytes = [0u8; 4];
                        val_bytes.copy_from_slice(&resp[6 + i * 4..10 + i * 4]);
                        result_matrix.push(u32::from_be_bytes(val_bytes));
                    }

                    println!("Result matrix:");
                    print_matrix(&result_matrix, recv_n);
                    break;
                }
            } else if status == ClientStatus::Running as u8 {
                println!("Status: In progress...");
                thread::sleep(time::Duration::from_millis(POLL_INTERVAL_MS));
            } else if status == ClientStatus::Error as u8 {
                println!("Status: Error on server");
                break;
            }
        }
    }

    Ok(())
}

fn send_msg(stream: &mut TcpStream, payload: &[u8]) -> std::io::Result<()> {
    let len = payload.len() as u32;
    stream.write_all(&len.to_be_bytes())?;
    stream.write_all(payload)?;
    Ok(())
}

fn recv_msg(stream: &mut TcpStream) -> std::io::Result<Vec<u8>> {
    let mut len_bytes = [0u8; 4];
    stream.read_exact(&mut len_bytes)?;
    let len = u32::from_be_bytes(len_bytes);

    let mut payload = vec![0u8; len as usize];
    if len > 0 {
        stream.read_exact(&mut payload)?;
    }
    Ok(payload)
}

fn print_matrix(matrix: &[u32], n: u32) {
    for i in 0..n {
        for j in 0..n {
            print!("{} ", matrix[(i * n + j) as usize]);
        }
        println!();
    }
}
