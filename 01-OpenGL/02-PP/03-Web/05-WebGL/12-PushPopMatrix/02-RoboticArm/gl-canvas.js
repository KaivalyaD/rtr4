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

// uniforms
var uniforms = {
    modelMatrix : null,
    viewMatrix : null,
    projMatrix : null,
    Ld : null,
    lightPosition: null,
    Kd : null,
    isOn : false
};

var light = {
    ambient : null,
    diffuse : null,
    specular : null,
    position : null,
    isLight : null
};
var material = {
    ambient : null,
    diffuse : null,
    specular : null,
    shininess: null
};

// for the App
var sphereObj = null;
var vaoCuboids;
var vboCuboidPositions;
var vboCuboidNormals;

var ltStack = null;
var shoulder_yaw = 0.0, shoulder_pitch = 0.0;
var elbow_pitch = 0.0, elbow_roll = 0.0;
var wrist_pitch = 0.0, wrist_roll = 0.0;
var finger_a = 0.0, finger_b = 0.0;
var perspectiveProjectionMatrix = null;

// a class to generate spheres
class sphere {
    #vao;
    #vbo;
    #eabo;
    #nslices;
    #nstacks;
    #count;

    constructor(slices, stacks) {
        this.#nslices = slices;
        if(this.#nslices <= 0)
            this.#nslices = 1;
        
        this.#nstacks = stacks;
        if(this.#nstacks <= 0)
            this.#nstacks = 1;
    }

    init(positionAttribLocation, normalAttribLocation) {
        var stacks = this.#nstacks;
        var slices = this.#nslices;

        var vertexData = []
		for(var i = 0; i <= stacks; i++) {
			var phi = Math.PI * i / stacks
			for(var j = 0; j <= slices; j++) {
				var theta = 2.0 * Math.PI * j / slices
				vertexData.push(Math.sin(phi) * Math.sin(theta))
				vertexData.push(Math.cos(phi))
				vertexData.push(Math.sin(phi) * Math.cos(theta))
				
				vertexData.push(Math.sin(phi) * Math.sin(theta))
				vertexData.push(Math.cos(phi))
				vertexData.push(Math.sin(phi) * Math.cos(theta))
			}
		}
		var elements = []
		for(var i = 0; i < stacks; i++) {
			var e1 = i * (slices + 1)
			var e2 = e1 + slices + 1
			for(var j = 0; j < slices; j++, e1++, e2++) {
				if(i != 0) {
					elements.push(e1)	
					elements.push(e2)
					elements.push(e1 + 1)
				}
				if(i != (stacks - 1)) {
					elements.push(e1 + 1)
					elements.push(e2)
					elements.push(e2 + 1)
				}
			}
		}
		var elementIndices = Uint16Array.from(elements)
		var vertexArray = Float32Array.from(vertexData)
        this.#count = elementIndices.length;

		var vao = gl.createVertexArray()
		gl.bindVertexArray(vao)
	
		var vbo = gl.createBuffer()
		gl.bindBuffer(gl.ARRAY_BUFFER, vbo)
		gl.bufferData(gl.ARRAY_BUFFER, vertexArray, gl.STATIC_DRAW)
		gl.vertexAttribPointer(positionAttribLocation, 3, gl.FLOAT, false, 6 * 4, 0)
		gl.enableVertexAttribArray(positionAttribLocation)
		gl.vertexAttribPointer(normalAttribLocation, 3, gl.FLOAT, false, 6 * 4, 3 * 4)
		gl.enableVertexAttribArray(normalAttribLocation)
        this.#vbo = vbo;
		
		var eabo = gl.createBuffer()
		gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, eabo)
		gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, elementIndices, gl.STATIC_DRAW)
        this.#eabo = eabo;
	
		gl.bindVertexArray(null)
        this.#vao = vao;
    }

    render() {
        gl.bindVertexArray(this.#vao)
        gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, this.#eabo)
        gl.drawElements(gl.TRIANGLES, this.#count, gl.UNSIGNED_SHORT, 0)
        gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, null)
		gl.bindVertexArray(null)
    }

    uninit() {
        if(this.#eabo) {
            gl.deleteBuffer(this.#eabo);
            this.#eabo = null;
        }
        if(this.#vbo) {
            gl.deleteBuffer(this.#vbo);
            this.#vbo = null;
        }
        if(this.#vao) {
            gl.deleteVertexArray(this.#vao);
            this.#vao = null;
        }
    }
}

// a (very trivial) stack of 4x4 linear transformations
class LTStack {
    #list;
    #top;
    #identity;
    constructor() {
        this.#list = [];
        this.#top = -1;
        this.#identity = mat4.create();
        mat4.identity(this.#identity);
    }
    push(mat) {
        var pushMat = mat4.create();
        mat4.multiply(pushMat, this.top(), mat);

        this.#top += 1;
        this.#list[this.#top] = pushMat;
    }
    pop() {
        if(this.#top < 0) {
            return;
        }
        this.#list[this.#top] = null;
        this.#top -= 1;
    }
    top() {
        if(this.#top < 0) {
            return this.#identity;
        }
        return this.#list[this.#top];
    }
}

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
    var min = function(x, y) {
        return x <= y ? x : y;
    }
    var max = function(x, y) {
        return x >= y ? x : y;
    }

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

    case 49:
        if(!event.shiftKey) {  // 1
            shoulder_yaw = (shoulder_yaw + 3.0);
        }
        else {  // !
            shoulder_yaw = (shoulder_yaw - 3.0);
        }
        break;

    case 50:
        if(!event.shiftKey) {  // 2
            shoulder_pitch = (shoulder_pitch + 3.0);
        }
        else {  // @
            shoulder_pitch = (shoulder_pitch - 3.0);
        }
        break;

    case 51:
        if(!event.shiftKey) {  // 3
            elbow_pitch = (elbow_pitch + 3.0);
        }
        else {  // #
            elbow_pitch = (elbow_pitch - 3.0);
        }
        break;

    case 52:
        if(!event.shiftKey) {  // 4
            elbow_roll = (elbow_roll + 3.0);
        }
        else {  // $
            elbow_roll = (elbow_roll - 3.0);
        }
        break;

    case 53:
        if(!event.shiftKey) {  // 5
            wrist_pitch = (wrist_pitch + 3.0);
        }
        else {  // %
            wrist_pitch = (wrist_pitch - 3.0);
        }
        break;

    case 54:
        if(!event.shiftKey) {  // 6
            wrist_roll = (wrist_roll + 3.0);
        }
        else {  // ^
            wrist_roll = (wrist_roll - 3.0);
        }
        break;

    case 55:
        if(!event.shiftKey) {  // 7
            finger_a = min(finger_a + 0.001, 0.04);
        }
        else {  // &
            finger_a = max(finger_a - 0.001, 0.0);
        }
        break;

    case 56:
        if(!event.shiftKey) {  // 8
            finger_b = min(finger_b + 0.001, 0.04);
        }
        else {  // *
            finger_b = max(finger_b - 0.001, 0.0);
        }
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
    in vec3 a_normal;

    uniform mat4 modelMatrix;
    uniform mat4 viewMatrix;
    uniform mat4 projMatrix;

    uniform vec4 Ld;
    uniform vec4 lightPosition;  // in eye coordinates
    uniform vec4 Kd;
    uniform int isLight;

    out vec4 a_color_out;

    void main(void)
    {
        mat4 mvMatrix = viewMatrix * modelMatrix;
        vec4 eyeCoordinate = mvMatrix * a_position;
        if(isLight != 0) {
            mat3 normalMatrix = mat3(transpose(inverse(mvMatrix)));
            vec3 normal = normalize(normalMatrix * a_normal);
            vec3 lightDirection = normalize(lightPosition.xyz - eyeCoordinate.xyz);
            a_color_out = Ld * Kd * max(dot(normal, lightDirection), 0.0);
        }
        else {
            a_color_out = vec4(1.0);
        }
        gl_Position = projMatrix * eyeCoordinate;
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
    gl.bindAttribLocation(shaderProgramObject, KVD.ATTRIBUTE_POSITION, "a_position");  // perform pre-linking rituals
    gl.bindAttribLocation(shaderProgramObject, KVD.ATTRIBUTE_NORMAL, "a_normal");  // perform pre-linking rituals
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
    uniforms.modelMatrix = gl.getUniformLocation(shaderProgramObject, "modelMatrix");
    uniforms.viewMatrix = gl.getUniformLocation(shaderProgramObject, "viewMatrix");
    uniforms.projMatrix = gl.getUniformLocation(shaderProgramObject, "projMatrix");
    uniforms.Ld = gl.getUniformLocation(shaderProgramObject, "Ld");
    uniforms.lightPosition = gl.getUniformLocation(shaderProgramObject, "lightPosition");
    uniforms.Kd = gl.getUniformLocation(shaderProgramObject, "Kd");
    uniforms.isLight = gl.getUniformLocation(shaderProgramObject, "isLight");

    // generate a sphere & put geometry data into rendering pipeline
    sphereObj = new sphere(25, 25);
    sphereObj.init(KVD.ATTRIBUTE_POSITION, KVD.ATTRIBUTE_NORMAL);

    // generate cuboids
    var cuboidDimensions = [
    //    w,   h,   d      //
        [0.1, 1.5, 0.1],   // forearm
        [0.1, 1.0, 0.1],   // arm
        [0.2, 0.2, 0.1],   // wrist
        [0.05, 0.1, 0.1]   // fingers
    ];
    vaoCuboids = generateCuboids(vboCuboidPositions, vboCuboidNormals, cuboidDimensions);

    // instantiate a stack for render() to use
    ltStack = new LTStack();

    // create and initialize perspective projection matrix
    perspectiveProjectionMatrix = mat4.create();
    mat4.identity(perspectiveProjectionMatrix);

    // setup light params
    light.diffuse = [1.0, 1.0, 1.0, 1.0];
    light.position = [0.0, 0.0, 5.0, 1.0];

    // setup material params
    material.diffuse = [0.5, 0.5, 0.5, 1.0];

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
    // local variables
    var modelMatrix = mat4.create();

    // code
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

    gl.useProgram(shaderProgramObject);
    {
        // set up lighting, material, view and projection matrix uniforms
        gl.uniformMatrix4fv(uniforms.viewMatrix, false, mat4.create());
        gl.uniformMatrix4fv(uniforms.projMatrix, false, perspectiveProjectionMatrix);
        gl.uniform4fv(uniforms.Ld, light.diffuse);
        gl.uniform4fv(uniforms.lightPosition, light.position);
        gl.uniform4fv(uniforms.Kd, material.diffuse);
        gl.uniform1i(uniforms.isLight, 1);

        // push initial transformation
        mat4.identity(modelMatrix);
        mat4.translate(modelMatrix, modelMatrix, [0.0, 0.0, -4.0]);
        ltStack.push(modelMatrix);
        {
            // push arm transformation
            mat4.identity(modelMatrix);
            mat4.translate(modelMatrix, modelMatrix, [0.0, 1.5, 0.0]);
            mat4.rotate(modelMatrix, modelMatrix, shoulder_yaw * Math.PI / 180.0, [0.0, 1.0, 0.0]);
            mat4.rotate(modelMatrix, modelMatrix, shoulder_pitch * Math.PI / 180.0, [1.0, 0.0, 0.0]);
            ltStack.push(modelMatrix);
            {
                // draw the arm
                gl.uniformMatrix4fv(uniforms.modelMatrix, false, ltStack.top());
                gl.bindVertexArray(vaoCuboids);
                    gl.drawArrays(gl.TRIANGLE_FAN, 0, 4);
                    gl.drawArrays(gl.TRIANGLE_FAN, 4, 4);
                    gl.drawArrays(gl.TRIANGLE_FAN, 8, 4);
                    gl.drawArrays(gl.TRIANGLE_FAN, 12, 4);
                    gl.drawArrays(gl.TRIANGLE_FAN, 16, 4);
                    gl.drawArrays(gl.TRIANGLE_FAN, 20, 4);
                gl.bindVertexArray(null);

                // push forearm transformation
                mat4.identity(modelMatrix);
                mat4.translate(modelMatrix, modelMatrix, [0.0, -1.5, 0.0]);
                mat4.rotate(modelMatrix, modelMatrix, elbow_pitch * Math.PI / 180.0, [0.0, 1.0, 0.0]);
                mat4.rotate(modelMatrix, modelMatrix, elbow_roll * Math.PI / 180.0, [1.0, 0.0, 0.0]);
                ltStack.push(modelMatrix);
                {
                    // draw the forearm
                    gl.uniformMatrix4fv(uniforms.modelMatrix, false, ltStack.top());
                    gl.bindVertexArray(vaoCuboids);
                        gl.drawArrays(gl.TRIANGLE_FAN, 24, 4);
                        gl.drawArrays(gl.TRIANGLE_FAN, 28, 4);
                        gl.drawArrays(gl.TRIANGLE_FAN, 32, 4);
                        gl.drawArrays(gl.TRIANGLE_FAN, 36, 4);
                        gl.drawArrays(gl.TRIANGLE_FAN, 40, 4);
                        gl.drawArrays(gl.TRIANGLE_FAN, 44, 4);
                    gl.bindVertexArray(null);

                    // push wrist transformation
                    mat4.identity(modelMatrix);
                    mat4.translate(modelMatrix, modelMatrix, [0.0, -1.0, 0.0]);
                    mat4.rotate(modelMatrix, modelMatrix, wrist_pitch * Math.PI / 180.0, [0.0, 1.0, 0.0]);
                    mat4.rotate(modelMatrix, modelMatrix, wrist_roll * Math.PI / 180.0, [1.0, 0.0, 0.0]);
                    ltStack.push(modelMatrix);
                    {
                        // draw the wrist joint
                        var tempScale = mat4.create();
                        mat4.scale(tempScale, tempScale, [0.05, 0.05, 0.05]);
                        ltStack.push(tempScale);  // temp push
                        {
                            gl.uniformMatrix4fv(uniforms.modelMatrix, false, ltStack.top());
                            sphereObj.render();
                        }
                        ltStack.pop();  // temp pop
                        
                        // draw the wrist
                        gl.uniformMatrix4fv(uniforms.modelMatrix, false, ltStack.top());
                        gl.bindVertexArray(vaoCuboids);
                            gl.drawArrays(gl.TRIANGLE_FAN, 48, 4);
                            gl.drawArrays(gl.TRIANGLE_FAN, 52, 4);
                            gl.drawArrays(gl.TRIANGLE_FAN, 56, 4);
                            gl.drawArrays(gl.TRIANGLE_FAN, 60, 4);
                            gl.drawArrays(gl.TRIANGLE_FAN, 64, 4);
                            gl.drawArrays(gl.TRIANGLE_FAN, 68, 4);
                        gl.bindVertexArray(null);

                        // push finger-a transformation
                        mat4.identity(modelMatrix);
                        mat4.translate(modelMatrix, modelMatrix, [-0.08 + finger_a, -0.2, 0.0]);
                        ltStack.push(modelMatrix);
                        {
                            // draw the wrist
                            gl.uniformMatrix4fv(uniforms.modelMatrix, false, ltStack.top());
                            gl.bindVertexArray(vaoCuboids);
                                gl.drawArrays(gl.TRIANGLE_FAN, 72, 4);
                                gl.drawArrays(gl.TRIANGLE_FAN, 76, 4);
                                gl.drawArrays(gl.TRIANGLE_FAN, 80, 4);
                                gl.drawArrays(gl.TRIANGLE_FAN, 84, 4);
                                gl.drawArrays(gl.TRIANGLE_FAN, 88, 4);
                                gl.drawArrays(gl.TRIANGLE_FAN, 92, 4);
                            gl.bindVertexArray(null);
                        }
                        ltStack.pop();  // pop finger-a transformation

                        // push finger-b transformation
                        mat4.identity(modelMatrix);
                        mat4.translate(modelMatrix, modelMatrix, [0.08 - finger_b, -0.2, 0.0]);
                        ltStack.push(modelMatrix);
                        {
                            // draw the wrist
                            gl.uniformMatrix4fv(uniforms.modelMatrix, false, ltStack.top());
                            gl.bindVertexArray(vaoCuboids);
                                gl.drawArrays(gl.TRIANGLE_FAN, 72, 4);
                                gl.drawArrays(gl.TRIANGLE_FAN, 76, 4);
                                gl.drawArrays(gl.TRIANGLE_FAN, 80, 4);
                                gl.drawArrays(gl.TRIANGLE_FAN, 84, 4);
                                gl.drawArrays(gl.TRIANGLE_FAN, 88, 4);
                                gl.drawArrays(gl.TRIANGLE_FAN, 92, 4);
                            gl.bindVertexArray(null);
                        }
                        ltStack.pop();  // pop finger-b transformation
                    }
                    ltStack.pop();  // pop wrist transformation

                    // draw the elbow
                    var tempScale = mat4.create();
                    mat4.scale(tempScale, tempScale, [0.1, 0.1, 0.1]);
                    ltStack.push(tempScale);  // temp push
                    {
                        gl.uniformMatrix4fv(uniforms.modelMatrix, false, ltStack.top());
                        sphereObj.render();
                    }
                    ltStack.pop();  // temp pop
                }
                ltStack.pop();  // pop forearm transformation

                // draw the shoulder
                var tempScale = mat4.create();
                mat4.scale(tempScale, tempScale, [0.1, 0.1, 0.1]);
                ltStack.push(tempScale);  // temp push
                {
                    gl.uniformMatrix4fv(uniforms.modelMatrix, false, ltStack.top());
                    sphereObj.render();
                }
                ltStack.pop();  // temp pop
            }
            ltStack.pop();  // pop arm transformation
        }
        ltStack.pop();  // pop initial transformation
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
    
    if(sphereObj) {
        sphereObj.uninit();
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

function generateCuboids(vboPositions, vboNormals, dimensions)
{
    // variable declarations
    var retVao = 0;

    // code
    retVao = gl.createVertexArray();
    gl.bindVertexArray(retVao);
    {
        // positions vbo
        vboPositions = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, vboPositions);
        {
            // allocate a dynamic data store for each cuboid having 24 3D vectors of 4-byte floating point values
            gl.bufferData(gl.ARRAY_BUFFER, dimensions.length * 24 * 3 * 4, gl.DYNAMIC_DRAW);
            gl.vertexAttribPointer(KVD.ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 0, 0);

            for(let i = 0; i < dimensions.length; i++)
            {
                var currentWidth = dimensions[i][0];
                var currentHeight = dimensions[i][1];
                var currentDepth = dimensions[i][2];

                var faceVerticesArray = Float32Array.from([
                    // front
                    currentWidth / 2.0,  0.0,  currentDepth / 2.0,
                   -currentWidth / 2.0,  0.0,  currentDepth / 2.0,
                   -currentWidth / 2.0, -currentHeight,  currentDepth / 2.0,
                    currentWidth / 2.0, -currentHeight,  currentDepth / 2.0,

                    // right
                    currentWidth / 2.0,  0.0, -currentDepth / 2.0,
                    currentWidth / 2.0,  0.0,  currentDepth / 2.0,
                    currentWidth / 2.0, -currentHeight,  currentDepth / 2.0,
                    currentWidth / 2.0, -currentHeight, -currentDepth / 2.0,

                    // back
                   -currentWidth / 2.0,  0.0, -currentDepth / 2.0,
                    currentWidth / 2.0,  0.0, -currentDepth / 2.0,
                    currentWidth / 2.0, -currentHeight, -currentDepth / 2.0,
                   -currentWidth / 2.0, -currentHeight, -currentDepth / 2.0,

                    // left
                   -currentWidth / 2.0,  0.0,  currentDepth / 2.0,
                   -currentWidth / 2.0,  0.0, -currentDepth / 2.0,
                   -currentWidth / 2.0, -currentHeight, -currentDepth / 2.0,
                   -currentWidth / 2.0, -currentHeight,  currentDepth / 2.0,

                    // top
                    currentWidth / 2.0,  0.0,  currentDepth / 2.0,
                    currentWidth / 2.0,  0.0, -currentDepth / 2.0,
                   -currentWidth / 2.0,  0.0, -currentDepth / 2.0,
                   -currentWidth / 2.0,  0.0,  currentDepth / 2.0,

                    // bottom
                    currentWidth / 2.0, -currentHeight,  currentDepth / 2.0,
                   -currentWidth / 2.0, -currentHeight,  currentDepth / 2.0,
                   -currentWidth / 2.0, -currentHeight, -currentDepth / 2.0,
                    currentWidth / 2.0, -currentHeight, -currentDepth / 2.0
                ]);

                gl.bufferSubData(gl.ARRAY_BUFFER, i * 24 * 3 * faceVerticesArray.BYTES_PER_ELEMENT, faceVerticesArray);
            }

            // copy data to vertex shader now
            gl.enableVertexAttribArray(KVD.ATTRIBUTE_POSITION);
        }
        gl.bindBuffer(gl.ARRAY_BUFFER, null);

        // normals vbo
        vboNormals = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, vboNormals);
        {
            // allocate a dynamic data store for each cuboid having 24 3D vectors of 4-byte floating point values
            gl.bufferData(gl.ARRAY_BUFFER, dimensions.length * 24 * 3 * 4, gl.DYNAMIC_DRAW);
            gl.vertexAttribPointer(KVD.ATTRIBUTE_NORMAL, 3, gl.FLOAT, false, 0, 0);

            for(let i = 0; i < dimensions.length; i++)
            {
                var faceNormalsArray = Float32Array.from([
                    // front
                    0.0,  0.0,  1.0,
                    0.0,  0.0,  1.0,
                    0.0,  0.0,  1.0,
                    0.0,  0.0,  1.0,

                    // right
                    1.0,  0.0,  0.0,
                    1.0,  0.0,  0.0,
                    1.0,  0.0,  0.0,
                    1.0,  0.0,  0.0,

                    // back
                    0.0,  0.0, -1.0,
                    0.0,  0.0, -1.0,
                    0.0,  0.0, -1.0,
                    0.0,  0.0, -1.0,

                    // left
                   -1.0,  0.0,  0.0,
                   -1.0,  0.0,  0.0,
                   -1.0,  0.0,  0.0,
                   -1.0,  0.0,  0.0,

                    // top
                    0.0,  1.0,  0.0,
                    0.0,  1.0,  0.0,
                    0.0,  1.0,  0.0,
                    0.0,  1.0,  0.0,

                    // bottom
                    0.0, -1.0, 0.0,
                    0.0, -1.0, 0.0,
                    0.0, -1.0, 0.0,
                    0.0, -1.0, 0.0
                ]);

                gl.bufferSubData(gl.ARRAY_BUFFER, i * 24 * 3 * faceNormalsArray.BYTES_PER_ELEMENT, faceNormalsArray);
            }

            gl.enableVertexAttribArray(KVD.ATTRIBUTE_NORMAL);
        }
        gl.bindBuffer(gl.ARRAY_BUFFER, null);
    }
    gl.bindVertexArray(null);

    return retVao;
}
