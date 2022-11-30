var canvas = null;
var isFullscreen = false;

function main() {
    // code
    // obtain canvas from html document
    canvas = document.getElementById("KVD");  // document is a builtin variable in DOM
    if(!canvas)
        console.log("main(): failed to obtain canvas from canvas.html\n");
    else
        console.log("main(): successfully obtained canvas from canvas.html\n");
    
    // obtain 2D context from canvas
    context = canvas.getContext("2d");
    if(!context)
        console.log("main(): failed to obtain context from canvas\n");
    else
        console.log("main(): successfully obtained context from canvas\n");

    // paint the canvas black
    context.fillStyle = "black";  // instead of "black", specifying hexcode as "#000000" works too
    context.fillRect(0, 0, canvas.width, canvas.height);

    // render text to canvas
    context.font = "48px sans-serif";
    context.fillStyle = "#00ff00";
    context.textAlign = "center";  // horizontal center alignment
    context.textBaseline = "middle";  // verticle center alignment
    context.fillText("Hello World!!!", canvas.width / 2, canvas.height / 2);

    // add keyboard and mouse event listeners
    window.addEventListener("keydown", keyDown, false);  // the last param decides whether we break(true) or return(false) after a message handler in WndProc()
    window.addEventListener("click", mouseDown, false);
}

/* keyboard event listener */
function keyDown(event) {
    // code
    switch(event.keyCode)
    {
    case 70:  // F
    case 102: // f
        toggleFullscreen();
        break;
    
    default:
        break;
    }
}

/* mouse event listener */
function mouseDown() {
    // code
}

/* fullscreen toggler */
function toggleFullscreen() {
    // code
    // 
    var fullscreen_element =
        document.fullscreenElement ||         // fullscreen support from chrome/opera
        document.mozFullScreenElement ||      // fullscreen support from firefox
        document.webkitFullscreenElement ||   // fullscreen support from safari
        document.msFullscreenElement ||       // fullscreen support from ie/edge
        null;                                // we are already fullscreen

    if(fullscreen_element == null)
    {
        if(canvas.requestFullscreen)
            canvas.requestFullscreen();
        else if(canvas.mozRequestFullScreen)
            canvas.mozRequestFullScreen();
        else if(canvas.webkitRequestFullscreen)
            canvas.webkitRequestFullscreen();
        else if(canvas.msRequestFullscreen)
            canvas.msRequestFullscreen();
        
        isFullscreen = true;
    }
    else
    {
        if(document.exitFullscreen)
            document.exitFullscreen();
        else if(document.mozExitFullScreen)
            document.mozExitFullScreen();
        else if(document.webkitExitFullscreen)
            document.webkitExitFullscreen();
        else if(document.msExitFullscreen)
            document.msExitFullscreen();

        isFullscreen = false;
    }
}
