/*
 * Copyright 2021, alex at staticlibs.net
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#[allow(non_snake_case)]
#[derive(serde_derive::Serialize, serde_derive::Deserialize)]
struct Meta {
    requestHandle: i64,
    uri: std::string::String,
    args: std::string::String,
    unparsedUri: std::string::String,
    method: std::string::String,
    protocol: std::string::String
}

#[derive(serde_derive::Serialize, serde_derive::Deserialize)]
struct Data {
    utf8: std::option::Option<std::string::String>,
    hex: std::option::Option<std::string::String>,
    file: std::option::Option<std::string::String>
}

#[derive(serde_derive::Serialize, serde_derive::Deserialize)]
struct Request {
    meta: Meta,
    headers: std::collections::HashMap<String, String>,
    data: Data
}

#[no_mangle]
pub extern "C"
fn submit_json_request(req_json: *const std::os::raw::c_char) -> std::os::raw::c_int {
    let c_str: &std::ffi::CStr = unsafe { std::ffi::CStr::from_ptr(req_json) };
    let json: Request = serde_json::from_slice(c_str.to_bytes()).unwrap();
    std::thread::spawn(move || {
        eprintln!("Hello from the app 2!");
        let st = serde_json::to_string_pretty(&json).unwrap();
        //eprintln!("{}", st);
        let client = reqwest::blocking::Client::new();
        match client.post("http://127.0.0.1:80/test_response")
                .body(st)
                .header("X-Nginx-Request-Handle", json.meta.requestHandle.to_string())
                .send() {
            Ok(_) => (),
            Err(e) => {
                eprintln!("{}", String::from(e.to_string()))
            }
        }
        eprintln!("Goodbye from the app!");
    });
    return 0;
}