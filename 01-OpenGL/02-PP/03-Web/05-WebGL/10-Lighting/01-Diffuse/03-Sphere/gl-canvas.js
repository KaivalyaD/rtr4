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

// lighting
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
var perspectiveProjectionMatrix;

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

    case 76:  // L
    case 108: // l
        light.isOn = !light.isOn;
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
        // perform transformations
        mat4.identity(modelMatrix);
        mat4.translate(modelMatrix, modelMatrix, [0.0, 0.0, -4.5]);

        // set up uniforms
        gl.uniformMatrix4fv(uniforms.modelMatrix, false, modelMatrix);
        gl.uniformMatrix4fv(uniforms.viewMatrix, false, mat4.create());
        gl.uniformMatrix4fv(uniforms.projMatrix, false, perspectiveProjectionMatrix);
        gl.uniform4fv(uniforms.Ld, light.diffuse);
        gl.uniform4fv(uniforms.lightPosition, light.position);
        gl.uniform4fv(uniforms.Kd, material.diffuse);
        gl.uniform1i(uniforms.isLight, light.isOn ? 1 : 0);

        sphereObj.render();
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
