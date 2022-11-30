function main() {
    // code
    // obtain canvas from html document
    var canvas = document.getElementById("KVD");  // document is a builtin variable in DOM
    if(!canvas)
    {
        console.log("main(): failed to obtain canvas from canvas.html\n");
        return;
    }
    console.log("main(): successfully obtained canvas from canvas.html\n");
    
    // obtain 2D context from canvas
    var context = canvas.getContext("2d");
    if(!context)
    {
        console.log("main(): failed to obtain context from canvas\n");
        return;
    }
    console.log("main(): successfully obtained context from canvas\n");

    // paint the canvas black
    context.fillStyle = "black";  // instead of "black", specifying hexcode as "#000000" works too
    context.fillRect(0, 0, canvas.width, canvas.height);

    var str = "Hello World!!!";
    context.font = "48px sans-serif";
    context.fillStyle = "#00ff00";
    context.textAlign = "center";  // horizontal center alignment
    context.textBaseline = "middle";  // verticle center alignment
    context.fillText(str, canvas.width / 2, canvas.height / 2);

    // add keyboard and mouse event listeners
    window.addEventListener("keydown", keyDown, false);  // the last param decides whether we break(true) or return(false) after a message handler in WndProc()
    window.addEventListener("click", mouseDown, false);
}

/* keyboard event listener */
function keyDown(event) {
    // code
}

/* mouse event listener */
function mouseDown() {
    // code
}
