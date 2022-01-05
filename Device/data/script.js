var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
window.addEventListener('load', onLoad);

function onLoad(event) {
    initWebSocket();
}

function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}

function onOpen(event) {
    console.log('Connection opened');
    websocket.send("states");
}

function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}

function onMessage(event) {
    console.log(event.data);
    var myObj = event.data;
    if (myObj[0]=="0")
        document.getElementById("1").checked = false;
    else
        document.getElementById("1").checked = true;
    if (myObj[1]=="0")
        document.getElementById("2").checked = false;
    else
        document.getElementById("2").checked = true;

}

function toggleCheckbox(element) {
    console.log(element.id);
    websocket.send(element.id);
}