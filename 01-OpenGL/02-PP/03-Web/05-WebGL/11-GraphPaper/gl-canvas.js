var canvas = null;
var gl = null;
var isFullscreen = false;
var canvasOriginalWidth, canvasOriginalHeight;

// for uniformity among programs
const KVD = {
    ATTRIBUTE_POSITION : 0,
    ATTRIBUTE_COLOR : 1,
    ATTRIBUTE_NORMAL : 2,
    ATTRIBUTE_TEXTURE0 : 3
};

// for GL
var shaderProgramObject;

// for rendered objects
var vaoLine;
var vaoSquare;
var vaoCircle;
var vaoTriangle;

var vboLine;
var vboSquare;
var vboCircle;
var vboTriangle;

var showTriangle = true;
var showSquare = true;
var showCircle = true;
var showGrid = true;

var mvpMatrixUniform;
var perspectiveProjectionMatrix;

// requestAnimationFrame() (browser-specific buffer-swapping function)
var requestAnimationFrame =
    window.requestAnimationFrame ||     // google chrome
    window.mozRequestAnimationFrame ||  // firefox
    window.oRequestAnimationFrame ||    // opera
    window.msRequestAnimationFrame ||   // ie/edge
    window.webkitRequestAnimationFrame; // safari

function main() {
    // code
    canvas = document.getElementById("WebGLCanvas");
    if(!canvas)
        console.log("main(): failed to obtain canvas from gl-canvas.html\n");
    else
        console.log("main(): successfully obtained canvas from gl-canvas.html\n");

    // record canvas dimensions for future resize() calls
    canvasOriginalWidth = canvas.width;
    canvasOriginalHeight = canvas.height;

    initialize(); // initialize WebGL
    resize();     // warm-up resize()

    // first render() call (no game loop in JS, instead cleverly use that of the browser)
    render();
    
    // add listeners that allow event to bubble up the hierarchy
    window.addEventListener("keydown", keyDown, false);
    window.addEventListener("click", mouseDown, false);
    window.addEventListener("resize", resize, false);
}

/* keyboard event listener */
function keyDown(event) {
    // code
    switch(event.keyCode)
    {
    case 81:   // Q (esc is already used by browsers to toggle back from fullscreen mode)
    case 113:  // q
        uninitialize();
        window.close();  // try exiting the canvas' window (not all browsers like this)
        break;

    case 70:  // F
    case 102: // f
        toggleFullscreen();
        break;

    case 67:  // C
    case 99:  // c
        if (!showCircle)
            showCircle = true;
        else
            showCircle = false;
        break;

    case 83:  // T
    case 115:  // t
        if (!showSquare)
            showSquare = true;
        else
            showSquare = false;
        break;

    case 84:  // T
    case 116:  // t
        if (!showTriangle)
            showTriangle = true;
        else
            showTriangle = false;
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
        document.fullscreenElement ||         // chrome/opera
        document.mozFullScreenElement ||      // firefox
        document.webkitFullscreenElement ||   // safari
        document.msFullscreenElement ||       // ie/edge
        null;                                 // already fullscreen

    if(!fullscreen_element)
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

function initialize() {
    // variable declarations
    var status;
    var infoLog;

    // code
    // obtain WebGL 2.0 context
    gl = canvas.getContext("webgl2");
    if(!gl)
        console.log("initialize(): failed to obtain WebGL 2.0 context from canvas\n");
    else
        console.log("initialize(): successfully obtained WebGL 2.0 context from canvas\n");
    
    // set context viewport width & height: an original idiosynchracy of WebGL
    gl.viewportWidth = canvas.width;
    gl.viewportHeight = canvas.height;

    // log GL information
    logGLInfo();

    // vertex shader
    status = false;
    infoLog = "";

    var vertexShaderSource =
    `#version 300 es

    precision lowp int;
    precision mediump float;

    in vec4 a_position;
    in vec4 a_color;

    uniform mat4 u_mvpMatrix;

    out vec4 a_color_out;

    void main(void)
    {
        gl_Position = u_mvpMatrix * a_position;
        a_color_out = a_color;
    }`;

    var vertexShaderObject = gl.createShader(gl.VERTEX_SHADER);
    gl.shaderSource(vertexShaderObject, vertexShaderSource);
    gl.compileShader(vertexShaderObject);
    status = gl.getShaderParameter(vertexShaderObject, gl.COMPILE_STATUS);
    if(!status)
    {
        infoLog = gl.getShaderInfoLog(vertexShaderObject);
        console.log("initialize(): *** vertex shader compilation errors ***\n");
        console.log(infoLog);

        uninitialize();
    }
    else
        console.log("initialize(): vertex shader was compiled successfully\n");

    // fragment shader
    status = false;
    infoLog = "";

    var fragmentShaderSource = 
    `#version 300 es
        
    precision lowp int;
    precision mediump float;

    in vec4 a_color_out;

    out vec4 FragColor;

    void main(void)
    {
        FragColor = a_color_out;
    }`;

    var fragmentShaderObject = gl.createShader(gl.FRAGMENT_SHADER);
    gl.shaderSource(fragmentShaderObject, fragmentShaderSource);
    gl.compileShader(fragmentShaderObject);
    status = gl.getShaderParameter(fragmentShaderObject, gl.COMPILE_STATUS);
    if(!status)
    {
        infoLog = gl.getShaderInfoLog(fragmentShaderObject);
        console.log("initialize(): *** fragment shader compilation errors ***\n");
        console.log(infoLog);

        uninitialize();
    }
    else
        console.log("initialize(): fragment shader was compiled successfully\n");

    // shader program object
    status = false;
    infoLog = "";

    shaderProgramObject = gl.createProgram();
    gl.attachShader(shaderProgramObject, vertexShaderObject);
    gl.attachShader(shaderProgramObject, fragmentShaderObject);
    gl.bindAttribLocation(shaderProgramObject, KVD.ATTRIBUTE_POSITION, "a_position");
    gl.bindAttribLocation(shaderProgramObject, KVD.ATTRIBUTE_COLOR, "a_color");
    gl.linkProgram(shaderProgramObject);  // link the program
    status = gl.getProgramParameter(shaderProgramObject, gl.LINK_STATUS);
    if(!status)
    {
        infoLog = gl.getProgramInfoLog(shaderProgramObject);
        console.log("initialize(): *** there were linking errors ***\n");
        console.log(infoLog + "\n");

        uninitialize();
    }
    else
        console.log("initialize(): shader program was linked successfully\n");

    // perform post-linking rituals
    mvpMatrixUniform = gl.getUniformLocation(shaderProgramObject, "u_mvpMatrix");

    // put geometry data into rendering pipeline
    vaoLine = gl.createVertexArray();
    gl.bindVertexArray(vaoLine);
    {
        vboLine = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, vboLine);
        {
            const vertexPositionsTypedArray = new Float32Array([
                1.0, 0.0, 0.0,
               -1.0, 0.0, 0.0
            ]);

            gl.bufferData(gl.ARRAY_BUFFER, vertexPositionsTypedArray, gl.STATIC_DRAW);
            gl.vertexAttribPointer(KVD.ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 0, 0);
            gl.enableVertexAttribArray(KVD.ATTRIBUTE_POSITION);
        }
        gl.bindBuffer(gl.ARRAY_BUFFER, null);
    }
    gl.bindVertexArray(null);

    vaoTriangle = gl.createVertexArray();
    gl.bindVertexArray(vaoTriangle);
    {
        vboTriangle = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, vboTriangle);
        {
            const vertexPositionsTypedArray = new Float32Array([
                Math.sin(0.0), Math.cos(0.0), 0.0,
                Math.sin(4.0 * Math.PI / 3.0), Math.cos(4.0 * Math.PI / 3.0), 0.0,
                Math.sin(2.0 * Math.PI / 3.0), Math.cos(2.0 * Math.PI / 3.0), 0.0,
            ]);

            gl.bufferData(gl.ARRAY_BUFFER, vertexPositionsTypedArray, gl.STATIC_DRAW);
            gl.vertexAttribPointer(KVD.ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 0, 0);
            gl.enableVertexAttribArray(KVD.ATTRIBUTE_POSITION);
        }
        gl.bindBuffer(gl.ARRAY_BUFFER, null);
    }
    gl.bindVertexArray(null);

    vaoSquare = gl.createVertexArray();
    gl.bindVertexArray(vaoSquare);
    {
        vboSquare = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, vboSquare);
        {
            const vertexPositionsTypedArray = new Float32Array([
                1.0,  1.0, 0.0,
               -1.0,  1.0, 0.0,
               -1.0, -1.0, 0.0,
                1.0, -1.0, 0.0
            ]);

            gl.bufferData(gl.ARRAY_BUFFER, vertexPositionsTypedArray, gl.STATIC_DRAW);
            gl.vertexAttribPointer(KVD.ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 0, 0);
            gl.enableVertexAttribArray(KVD.ATTRIBUTE_POSITION);
        }
        gl.bindBuffer(gl.ARRAY_BUFFER, null);
    }
    gl.bindVertexArray(null);

    vaoCircle = gl.createVertexArray();
    gl.bindVertexArray(vaoCircle);
    {
        vboCircle = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, vboCircle);
        {
            var data = [];
            var size = 100;
            for (var i = 0; i < size; i++)
            {
                var theta = 2.0 * Math.PI * i / size;
                data.push(Math.cos(theta));
                data.push(Math.sin(theta));
                data.push(0.0);
            }
            const vertexPositionsTypedArray = new Float32Array(data);
            
            gl.bufferData(gl.ARRAY_BUFFER, vertexPositionsTypedArray, gl.STATIC_DRAW);
            gl.vertexAttribPointer(KVD.ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 0, 0);
            gl.enableVertexAttribArray(KVD.ATTRIBUTE_POSITION);
        }
        gl.bindBuffer(gl.ARRAY_BUFFER, null);
    }
    gl.bindVertexArray(null);

    // create and initialize perspective projection matrix
    perspectiveProjectionMatrix = mat4.create();
    mat4.identity(perspectiveProjectionMatrix);

    // set clear color
    gl.clearColor(0.0, 0.0, 0.0, 1.0);

    // enable depth testing
    gl.clearDepth(1.0);
    gl.enable(gl.DEPTH_TEST);
    gl.depthFunc(gl.LEQUAL);

    gl.enable(gl.CULL_FACE);
}

function logGLInfo() {
    // code
    document.getElementById("webgl_vendor_string").innerText = "Vendor: " + gl.getParameter(gl.VENDOR);
    document.getElementById("webgl_version_string").innerText = "Version: " + gl.getParameter(gl.VERSION);
    document.getElementById("webgl_sl_version_string").innerText = "GLSL Version: " + gl.getParameter(gl.SHADING_LANGUAGE_VERSION);
    document.getElementById("webgl_renderer_string").innerText = "Renderer: " + gl.getParameter(gl.RENDERER);
}

function resize() {
    // code
    if(isFullscreen)
    {
        canvas.width = window.innerWidth;
        canvas.height = window.innerHeight;
    }
    else
    {
        canvas.width = canvasOriginalWidth;
        canvas.height = canvasOriginalHeight;
    }

    gl.viewport(0, 0, canvas.width, canvas.height);

    let aspectRatio = canvas.width / canvas.height;
    mat4.perspective(perspectiveProjectionMatrix, Math.PI / 4, aspectRatio, 0.0, 100.0);
}

function render() {
    // code
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

    gl.useProgram(shaderProgramObject);
    {
        // set up uniforms
        var modelViewMatrix = mat4.create();
        var modelViewProjectionMatrix = mat4.create();

        var y = 1.0;
        for (var i = 0; i < 41; i++)
        {
            if (i != 20)
            {
                modelViewMatrix = mat4.create();
                modelViewProjectionMatrix = mat4.create();

                mat4.translate(modelViewMatrix, modelViewMatrix, [0.0, y, -3.0]);
                mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);

                gl.uniformMatrix4fv(mvpMatrixUniform, false, modelViewProjectionMatrix);
                gl.vertexAttrib4f(KVD.ATTRIBUTE_COLOR, 0.0, 0.0, 1.0, 1.0);

                gl.bindVertexArray(vaoLine);
                    gl.drawArrays(gl.LINES, 0, 2);
                gl.bindVertexArray(null);
            }
            y = y - (1.0 / 20.0);
        }

        var x = 1.0;
        for (var i = 0; i < 41; i++)
        {
            if (i != 20)
            {
                modelViewMatrix = mat4.create();
                modelViewProjectionMatrix = mat4.create();
        
                mat4.translate(modelViewMatrix, modelViewMatrix, [x, 0.0, -3.0]);
                mat4.rotate(modelViewMatrix, modelViewMatrix, Math.PI / 2.0, [0.0, 0.0, 1.0]);
                mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);

                gl.uniformMatrix4fv(mvpMatrixUniform, false, modelViewProjectionMatrix);
                gl.vertexAttrib4f(KVD.ATTRIBUTE_COLOR, 0.0, 0.0, 1.0, 1.0);

                gl.bindVertexArray(vaoLine);
                    gl.drawArrays(gl.LINES, 0, 2);
                gl.bindVertexArray(null);
            }

            x = x - (1.0 / 20.0);
        }

        // the X-Axis
        modelViewMatrix = mat4.create();
        modelViewProjectionMatrix = mat4.create();

        mat4.translate(modelViewMatrix, modelViewMatrix, [0.0, 0.0, -3.0]);
        mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);

        gl.uniformMatrix4fv(mvpMatrixUniform, false, modelViewProjectionMatrix);
        gl.vertexAttrib4f(KVD.ATTRIBUTE_COLOR, 1.0, 0.0, 0.0, 1.0);

        gl.bindVertexArray(vaoLine);
            gl.drawArrays(gl.LINES, 0, 2);
        gl.bindVertexArray(null);

        // the Y-Axis
        modelViewMatrix =mat4.create();
        modelViewProjectionMatrix = mat4.create();

        mat4.translate(modelViewMatrix, modelViewMatrix, [0.0, 0.0, -3.0]);
        mat4.rotate(modelViewMatrix, modelViewMatrix, Math.PI / 2.0, [0.0, 0.0, 1.0]);
        mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);

        gl.uniformMatrix4fv(mvpMatrixUniform, false, modelViewProjectionMatrix);
        gl.vertexAttrib4f(KVD.ATTRIBUTE_COLOR, 0.0, 1.0, 0.0, 1.0);

        gl.bindVertexArray(vaoLine);
            gl.drawArrays(gl.LINES, 0, 2);
        gl.bindVertexArray(null);

        // other objects
        if (showTriangle)
        {
            modelViewMatrix = mat4.create();
            modelViewProjectionMatrix = mat4.create();
            mat4.translate(modelViewMatrix, modelViewMatrix, [0.0, 0.0, -3.0]);
            mat4.scale(modelViewMatrix, modelViewMatrix, [0.7, 0.7, 0.7]);
            mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);

            gl.uniformMatrix4fv(mvpMatrixUniform, false, modelViewProjectionMatrix);
            gl.vertexAttrib4f(KVD.ATTRIBUTE_COLOR, 1.0, 1.0, 0.0, 1.0);
            
            gl.bindVertexArray(vaoTriangle);
                gl.drawArrays(gl.LINE_LOOP, 0, 3);
            gl.bindVertexArray(null);
        }

        if (showSquare)
        {
            modelViewMatrix = mat4.create();
            modelViewProjectionMatrix = mat4.create();
            mat4.translate(modelViewMatrix, modelViewMatrix, [0.0, 0.0, -3.0]);
            mat4.scale(modelViewMatrix, modelViewMatrix, [0.7, 0.7, 0.7]);
            mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);
            
            gl.uniformMatrix4fv(mvpMatrixUniform, false, modelViewProjectionMatrix);
            gl.vertexAttrib4f(KVD.ATTRIBUTE_COLOR, 0.8, 0.3, 0.0, 1.0);

            gl.bindVertexArray(vaoSquare);
                gl.drawArrays(gl.LINE_LOOP, 0, 4);
            gl.bindVertexArray(null);
        }

        if (showCircle)
        {
            modelViewMatrix = mat4.create();
            modelViewProjectionMatrix = mat4.create();
            mat4.translate(modelViewMatrix, modelViewMatrix, [0.0, 0.0, -3.0]);
            mat4.scale(modelViewMatrix, modelViewMatrix, [0.7, 0.7, 0.7]);
            mat4.multiply(modelViewProjectionMatrix, perspectiveProjectionMatrix, modelViewMatrix);

            gl.uniformMatrix4fv(mvpMatrixUniform, false, modelViewProjectionMatrix);
            gl.vertexAttrib4f(KVD.ATTRIBUTE_COLOR, 1.0, 0.0, 1.0, 1.0);

            gl.bindVertexArray(vaoCircle);
            gl.drawArrays(gl.LINE_LOOP, 0, 100);
            gl.bindVertexArray(null);
        }
    }
    gl.useProgram(null);

    // swap buffers and maintain the rendering loop
    requestAnimationFrame(render, canvas);

    // update the frame
    update();
}

function update() {
    // code
}

function uninitialize() {
    // code
    if(isFullscreen)
        toggleFullscreen();
    
    if(vboCircle != null)
    {
        gl.deleteBuffer(vboCircle);
        vboCircle = null;
    }

    if(vaoCircle != null)
    {
        gl.deleteVertexArray(vaoCircle);
        vaoCircle = null;
    }

    if(vboSquare != null)
    {
        gl.deleteBuffer(vboSquare);
        vboSquare = null;
    }

    if(vaoSquare != null)
    {
        gl.deleteVertexArray(vaoSquare);
        vaoSquare = null;
    }

    if(vboTriangle != null)
    {
        gl.deleteBuffer(vboTriangle);
        vboTriangle = null;
    }

    if(vaoTriangle != null)
    {
        gl.deleteVertexArray(vaoTriangle);
        vaoTriangle = null;
    }

    if(vboLine != null)
    {
        gl.deleteBuffer(vboLine);
        vboLine = null;
    }

    if(vaoLine != null)
    {
        gl.deleteVertexArray(vaoLine);
        vaoLine = null;
    }

    if(shaderProgramObject != null)
    {
        let attachedShadersCount = gl.getProgramParameter(shaderProgramObject, gl.ATTACHED_SHADERS);
        if(attachedShadersCount > 0)
        {
            let attachedShadersList = gl.getAttachedShaders(shaderProgramObject);
            let reversedAttachedShadersList = attachedShadersList.reverse();

            for(let i = 0; i < attachedShadersCount; i++)
            {
                gl.detachShader(shaderProgramObject, reversedAttachedShadersList[i]);
                gl.deleteShader(reversedAttachedShadersList[i]);
                reversedAttachedShadersList[i] = null;
            }
        }
        
        console.log("uninitialize(): detached and deleted " + attachedShadersCount + " shader objects\n");
        
        gl.deleteProgram(shaderProgramObject);
        shaderProgramObject = null;
    }
}
