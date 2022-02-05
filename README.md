Serverside Keyauth
====
This is way more secure than client side authentication if you implement it correctly and do some extra steps.

- - - -

### How to implement this correctly?
- Encrypt the packets
- Make the packets invalid after being used once
- Handle key parts of the application from the server (just like I sent the parameters for MessageBoxA in the example) so a cracker cannot just jump to a function in order to bypass the authentication

### Building

Prerequisites:
- A Linux server with the latest stable CMake and g++ installed
- A Windows machine with Visual Studio installed
- - - -
#### Building the server:
Clone the repository and create a build folder
```
cd build
cmake ..
cmake --build .
```
Now you can execute `server`
- - - -
#### Building the client:
Open the solution and click Build -> Rebuild Solution

### Example output
Client:

![](https://i.imgur.com/YaYT7OM.png)

Server:

![](https://i.imgur.com/U6xg2SZ.png)
