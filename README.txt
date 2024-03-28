Lab 1:
Abreham Getachew - getac021
Natan Teshome - tesho012

Contributions:
Abreham Getachew - Worked on each portion of lab equally.
Natan Teshome - Worked on each portion of lab equally.

Description:
This project is a simple HTTP web server developed in C, designed to comply with the basic principles of HTTP/1.0 as outlined in RFC 1945. 
The server supports handling GET requests for HTML and image files, 
using the standard C socket library for network communications. 
It's structured to serve as an educational tool, demonstrating the fundamentals of web server operation, 
including request handling, content delivery, and basic server configuration.

Files: 
- HTTP GET Requests: Supports handling HTTP GET requests for serving HTML and image content.
- Config File: Uses a `httpd.conf` file for server configuration, allowing customization of the server port, maximum connections, root directory, and default index file.

How to use:
1. Run the compiled executable (`myhttpserver`).
2. Access the server through a web browser or HTTP client, using the specified port and server address.

Config:
The server configuration is specified in the `httpd.conf` file. Below are the configurable parameters:
- `Port`: The port on which the server listens for incoming connections.
- `MaxConnections`: The maximum number of simultaneous connections the server can handle.
- `RootDirectory`: The root directory from which the server serves files.
- `DefaultIndex`: The default index file served when accessing the root URL.
