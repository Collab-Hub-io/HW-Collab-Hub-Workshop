# WAC2021 Collab-Hub Workshop
## Creating Telematic Musical Performances Through Max And The Web With Collab-Hub

### Using Collab-Hub in Web Audio Applications

Collab-Hub uses the [Socket.IO](https://socket.io/) library for client-server communications.

To use Collab-Hub in your web audio application, start by including Socket.IO.
Clone from [GitHub](https://github.com/socketio/socket.io), or install using npm:  
`npm install socket.io`  

Include in HTML head:  
`<script src="/socket.io/socket.io.js"></script>`  

Developers using only the client functionality of Socket.IO may alternatively install the smaller client-only package:
`npm install socket.io-client`

---
#### Messages
All control, event, and chat messages are sent as objects in the following formats. Client applications parse these objects as needed for native use in their respective hosts (e.g. Max, Pd, etc).

*Control:*  
```Javascript
("control", {
    header: "headerName",
    values: xyz,
    mode: "push" or "publish",
    target: "all" or "user" or "room"
};
```  

*Event:*  
```Javascript
("event", {
    header: "headerName",
    mode: "push" or "publish",
    target: "all" or "user" or "room"
};
```  

*Chat:*  
```Javascript
("chat", {
    chat: "Hello!",
    target: "all" or "user" or "room"
};
```  

Incoming messages from the server add the "from" property, identifying the sender.  

Example function to receive an incoming event message:
```Javascript
socket.on("event", incoming => {
    let sender = incoming.from;
    let header = incoming.header;
    if (header == "expectedHeader") {
      eventResponse();
    };
});
```

---
#### Additional Functionality
Other functions of Collab-Hub include adding/changing usernames, joining/leaving rooms, viewing list of connected users, etc. Documentation of implementation procedures for additional functionality in web audio applications is forthcoming, though much of this may be extrapolated from existing client source code e.g. [Max client script](https://github.com/Collab-Hub-io/WAC2021-Collab-Hub-Workshop/blob/main/Collab-Hub-Max-Client-v0.3/CH-ClientScript-v0.3.js).