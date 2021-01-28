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

#[no_mangle]
pub extern "C"
fn handle_json_request(_req_json: std::os::raw::c_char) -> std::os::raw::c_int {
    std::thread::spawn(|| {
        eprintln!("Hello from the app 2!");
        let client = reqwest::blocking::Client::new();
        client.post("http://127.0.0.1:80/test_response")
                .body("hi!!!")
                .send();
        eprintln!("Goodbye from the app!");
    });
    return 0;
}