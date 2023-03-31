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
var lightingProgram;
var textureProgram;

// uniforms
class LightingUniforms {
    modelMatrix;
    viewMatrix;
    projMatrix;
    La;
    Ld;
    Ls;
    lightPosition;
    Ka;
    Kd;
    Ks;
    materialShininess;
    viewerPosition;
    isLight;
    constructor() {
        this.La = [];
        this.Ld = [];
        this.Ls = [];
        this.lightPosition = [];
    }
};

class TextureUniforms {
    mvpMatrix;
    textureSampler;
}

// lighting
class Light {
    ambient;
    diffuse;
    specular;
    position;
    constructor() {
        this.ambient = [];
        this.diffuse = [];
        this.specular = [];
        this.position = [];
    }
};
var material = {
    ambient : null,
    diffuse : null,
    specular : null,
    shininess: null
};

// for the App
var vaoCube = null;
var vboCube = null;
var sphereObj = null;

var lightingUniforms = null;
var lights = [];
var isLight = false;

var thetaLights = 0.0;
var thetaCube = 0.0;

var projectionMatrix_Sphere = null;
var projectionMatrix_Cube = null;

var textureUniforms = null;

var fbo;
var texColor;
var rboDepth;
const texWidth = 512;
const texHeight = 512;

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

    initialize(); // initialize this App
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
        isLight = !isLight;
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

function initializeSphere() {
    // variable declarations
    var status;
    var infoLog;

    // code
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
    
    uniform vec4 lightPosition[3];  // in eye coordinates
    uniform vec4 viewerPosition; // in eye coordinates

    uniform int isLight;

    out vec3 a_normal_out;
    out vec3 a_lightDirection_out[3];
    out vec3 a_viewDirection_out;

    void main(void)
    {
        mat4 mvMatrix = viewMatrix * modelMatrix;
        vec4 eyeCoordinate = mvMatrix * a_position;
        if(isLight != 0) {
            mat3 normalMatrix = mat3(transpose(inverse(mvMatrix)));
            a_normal_out = normalMatrix * a_normal;
            a_lightDirection_out[0] = lightPosition[0].xyz - eyeCoordinate.xyz;
            a_lightDirection_out[1] = lightPosition[1].xyz - eyeCoordinate.xyz;
            a_lightDirection_out[2] = lightPosition[2].xyz - eyeCoordinate.xyz;
            a_viewDirection_out = viewerPosition.xyz - eyeCoordinate.xyz;
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
        console.log("initializeSphere(): *** vertex shader compilation errors ***\n");
        console.log(infoLog);

        uninitialize();
    }
    else
        console.log("initializeSphere(): vertex shader was compiled successfully\n");

    // fragment shader
    status = false;
    infoLog = "";

    var fragmentShaderSource = 
    `#version 300 es
    
    precision lowp int;
    precision mediump float;

    in vec3 a_normal_out;
    in vec3 a_lightDirection_out[3];
    in vec3 a_viewDirection_out;

    uniform vec4 La[3];
    uniform vec4 Ld[3];
    uniform vec4 Ls[3];

    uniform vec4 Ka;
    uniform vec4 Kd;
    uniform vec4 Ks;
    uniform float materialShininess;

    uniform int isLight;

    out vec4 FragColor;

    void main(void)
    {
        if(isLight != 0) {
            vec3 normal = normalize(a_normal_out);

            vec3 lightDirection[3];
            lightDirection[0] = normalize(a_lightDirection_out[0]);
            lightDirection[1] = normalize(a_lightDirection_out[1]);
            lightDirection[2] = normalize(a_lightDirection_out[2]);

            vec3 reflectedDirection[3];
            reflectedDirection[0] = reflect(-lightDirection[0], normal);
            reflectedDirection[1] = reflect(-lightDirection[1], normal);
            reflectedDirection[2] = reflect(-lightDirection[2], normal);

            vec3 viewDirection = normalize(a_viewDirection_out);

            vec4 ambient = (La[0] + La[1] + La[2]) * Ka;

            vec4 diffuse = vec4(0.0);
            diffuse += Ld[0] * Kd * max(dot(normal, lightDirection[0]), 0.0);
            diffuse += Ld[1] * Kd * max(dot(normal, lightDirection[1]), 0.0);
            diffuse += Ld[2] * Kd * max(dot(normal, lightDirection[2]), 0.0);
            
            vec4 specular = vec4(0.0);
            specular += Ls[0] * Ks * pow(max(dot(reflectedDirection[0], viewDirection), 0.0), materialShininess);
            specular += Ls[1] * Ks * pow(max(dot(reflectedDirection[1], viewDirection), 0.0), materialShininess);
            specular += Ls[2] * Ks * pow(max(dot(reflectedDirection[2], viewDirection), 0.0), materialShininess);

            FragColor = ambient + diffuse + specular;
        }
        else {
            FragColor = vec4(1.0);
        }
    }`;

    var fragmentShaderObject = gl.createShader(gl.FRAGMENT_SHADER);
    gl.shaderSource(fragmentShaderObject, fragmentShaderSource);
    gl.compileShader(fragmentShaderObject);
    status = gl.getShaderParameter(fragmentShaderObject, gl.COMPILE_STATUS);
    if(!status)
    {
        infoLog = gl.getShaderInfoLog(fragmentShaderObject);
        console.log("initializeSphere(): *** fragment shader compilation errors ***\n");
        console.log(infoLog);

        uninitialize();
    }
    else
        console.log("initializeSphere(): fragment shader was compiled successfully\n");

    // shader program object
    status = false;
    infoLog = "";

    lightingProgram = gl.createProgram();
    gl.attachShader(lightingProgram, vertexShaderObject);
    gl.attachShader(lightingProgram, fragmentShaderObject);
    gl.bindAttribLocation(lightingProgram, KVD.ATTRIBUTE_POSITION, "a_position");  // perform pre-linking rituals
    gl.bindAttribLocation(lightingProgram, KVD.ATTRIBUTE_NORMAL, "a_normal");  // perform pre-linking rituals
    gl.linkProgram(lightingProgram);  // link the program
    status = gl.getProgramParameter(lightingProgram, gl.LINK_STATUS);
    if(!status)
    {
        infoLog = gl.getProgramInfoLog(lightingProgram);
        console.log("initializeSphere(): *** there were linking errors ***\n");
        console.log(infoLog + "\n");

        uninitialize();
    }
    else
        console.log("initializeSphere(): shader program was linked successfully\n");

    // perform post-linking rituals
    lightingUniforms = new LightingUniforms();
    lightingUniforms.modelMatrix = gl.getUniformLocation(lightingProgram, "modelMatrix");
    lightingUniforms.viewMatrix = gl.getUniformLocation(lightingProgram, "viewMatrix");
    lightingUniforms.projMatrix = gl.getUniformLocation(lightingProgram, "projMatrix");
    lightingUniforms.La[0] = gl.getUniformLocation(lightingProgram, "La[0]");
    lightingUniforms.La[1] = gl.getUniformLocation(lightingProgram, "La[1]");
    lightingUniforms.La[2] = gl.getUniformLocation(lightingProgram, "La[2]");
    lightingUniforms.Ld[0] = gl.getUniformLocation(lightingProgram, "Ld[0]");
    lightingUniforms.Ld[1] = gl.getUniformLocation(lightingProgram, "Ld[1]");
    lightingUniforms.Ld[2] = gl.getUniformLocation(lightingProgram, "Ld[2]");
    lightingUniforms.Ls[0] = gl.getUniformLocation(lightingProgram, "Ls[0]");
    lightingUniforms.Ls[1] = gl.getUniformLocation(lightingProgram, "Ls[1]");
    lightingUniforms.Ls[2] = gl.getUniformLocation(lightingProgram, "Ls[2]");
    lightingUniforms.lightPosition[0] = gl.getUniformLocation(lightingProgram, "lightPosition[0]");
    lightingUniforms.lightPosition[1] = gl.getUniformLocation(lightingProgram, "lightPosition[1]");
    lightingUniforms.lightPosition[2] = gl.getUniformLocation(lightingProgram, "lightPosition[2]");
    lightingUniforms.Ka = gl.getUniformLocation(lightingProgram, "Ka");
    lightingUniforms.Kd = gl.getUniformLocation(lightingProgram, "Kd");
    lightingUniforms.Ks = gl.getUniformLocation(lightingProgram, "Ks");
    lightingUniforms.materialShininess = gl.getUniformLocation(lightingProgram, "materialShininess");
    lightingUniforms.viewerPosition = gl.getUniformLocation(lightingProgram, "viewerPosition");
    lightingUniforms.isLight = gl.getUniformLocation(lightingProgram, "isLight");

    // generate a sphere & put geometry data into rendering pipeline
    sphereObj = new sphere(25, 25);
    sphereObj.init(KVD.ATTRIBUTE_POSITION, KVD.ATTRIBUTE_NORMAL);

    // create and initialize perspective projection matrix
    projectionMatrix_Sphere = mat4.create();
    mat4.identity(projectionMatrix_Sphere);

    // setup light params
    lights[0] = new Light();
    lights[0].ambient = [0.0, 0.0, 0.0, 1.0];
    lights[0].diffuse = [1.0, 0.0, 0.0, 1.0];
    lights[0].specular = [1.0, 0.0, 0.0, 1.0];

    lights[1] = new Light();
    lights[1].ambient = [0.0, 0.0, 0.0, 1.0];
    lights[1].diffuse = [0.0, 1.0, 0.0, 1.0];
    lights[1].specular = [0.0, 1.0, 0.0, 1.0];

    lights[2] = new Light();
    lights[2].ambient = [0.0, 0.0, 0.0, 1.0];
    lights[2].diffuse = [0.0, 0.0, 1.0, 1.0];
    lights[2].specular = [0.0, 0.0, 1.0, 1.0];

    // setup material params
    material.ambient = [0.0, 0.0, 0.0, 1.0];
    material.diffuse = [0.5, 0.2, 0.7, 1.0];
    material.specular = [0.7, 0.7, 0.7, 1.0];
    material.shininess = 128.0;
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

    // create and configure the FBO
    fbo = gl.createFramebuffer();
    gl.bindFramebuffer(gl.FRAMEBUFFER, fbo);
    {
        // create a renderbuffer for depth attachment
        rboDepth = gl.createRenderbuffer();
        gl.bindRenderbuffer(gl.RENDERBUFFER, rboDepth);
        
        gl.renderbufferStorage(gl.RENDERBUFFER, gl.DEPTH_COMPONENT16, canvas.width, canvas.height);

        // associate rbo with fbo
        gl.framebufferRenderbuffer(gl.FRAMEBUFFER, gl.DEPTH_ATTACHMENT, gl.RENDERBUFFER, rboDepth);

        // creae a texture for color attachment
        texColor = gl.createTexture();
        gl.bindTexture(gl.TEXTURE_2D, texColor);

        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
        gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);

        gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGB, canvas.width, canvas.height, 0, gl.RGB, gl.UNSIGNED_SHORT_5_6_5, null);

        // associate tex with fbo
        gl.framebufferTexture2D(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, texColor, 0);

        var result = gl.checkFramebufferStatus(gl.FRAMEBUFFER);
        if(result != gl.FRAMEBUFFER_COMPLETE) {
            console.log("Framebuffer is not complete");
        }
    }
    gl.bindFramebuffer(gl.FRAMEBUFFER, null);

    // initialize the lighting program & generate a sphere
    initializeSphere();

    // shaders for the cube
    var vertexShaderSource =
    `#version 300 es

    precision lowp int;
    precision mediump float;

    in vec4 a_position;
    in vec2 a_texCoord;

    uniform mat4 u_mvpMatrix;

    out vec2 a_texCoord_out;

    void main(void)
    {
        gl_Position = u_mvpMatrix * a_position;
        a_texCoord_out = mat2(vec2(1.0, 0.0), vec2(0.0, -1.0)) * a_texCoord;
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

    in vec2 a_texCoord_out;

    uniform sampler2D u_textureSampler;

    out vec4 FragColor;

    void main(void)
    {
        FragColor = texture(u_textureSampler, a_texCoord_out);
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

    textureProgram = gl.createProgram();
    gl.attachShader(textureProgram, vertexShaderObject);
    gl.attachShader(textureProgram, fragmentShaderObject);
    gl.bindAttribLocation(textureProgram, KVD.ATTRIBUTE_POSITION, "a_position");
    gl.bindAttribLocation(textureProgram, KVD.ATTRIBUTE_TEXTURE0, "a_texCoord");
    gl.linkProgram(textureProgram);  // link the program
    status = gl.getProgramParameter(textureProgram, gl.LINK_STATUS);
    if(!status)
    {
        infoLog = gl.getProgramInfoLog(textureProgram);
        console.log("initialize(): *** there were linking errors ***\n");
        console.log(infoLog + "\n");

        uninitialize();
    }
    else
        console.log("initialize(): shader program was linked successfully\n");

    // perform post-linking rituals
    textureUniforms = new TextureUniforms();
    textureUniforms.mvpMatrix = gl.getUniformLocation(textureProgram, "u_mvpMatrix");
    textureUniforms.textureSampler = gl.getUniformLocation(textureProgram, "u_textureSampler");

    // put cube data into rendering pipeline
    vaoCube = gl.createVertexArray();
    gl.bindVertexArray(vaoCube);
    {
        // positions vbo
        vboCube = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, vboCube);
        {
            const vertexDataArray = Float32Array.from([
                // positions        // texCoords
                // front
                1.0,  1.0,  1.0,    1.0, 1.0,
               -1.0,  1.0,  1.0,    0.0, 1.0,
               -1.0, -1.0,  1.0,    0.0, 0.0,
                1.0, -1.0,  1.0,    1.0, 0.0,
                
                // right
                1.0,  1.0,  1.0,    1.0, 1.0,
                1.0, -1.0,  1.0,    0.0, 1.0,
                1.0, -1.0, -1.0,    0.0, 0.0,
                1.0,  1.0, -1.0,    1.0, 0.0,

                // rear
               -1.0,  1.0, -1.0,    1.0, 1.0,
                1.0,  1.0, -1.0,    0.0, 1.0,
                1.0, -1.0, -1.0,    0.0, 0.0,
               -1.0, -1.0, -1.0,    1.0, 0.0,

                // left
               -1.0,  1.0, -1.0,    1.0, 1.0,
               -1.0, -1.0, -1.0,    0.0, 1.0,
               -1.0, -1.0,  1.0,    0.0, 0.0,
               -1.0,  1.0,  1.0,    1.0, 0.0,

                // top
               -1.0,  1.0,  1.0,    1.0, 1.0,
                1.0,  1.0,  1.0,    0.0, 1.0,
                1.0,  1.0, -1.0,    0.0, 0.0,
               -1.0,  1.0, -1.0,    1.0, 0.0,

                // bottom
               -1.0, -1.0,  1.0,    1.0, 1.0,
               -1.0, -1.0, -1.0,    0.0, 1.0,
                1.0, -1.0, -1.0,    0.0, 0.0,
                1.0, -1.0,  1.0,    1.0, 0.0
            ]);

            gl.bufferData(gl.ARRAY_BUFFER, vertexDataArray, gl.STATIC_DRAW);
            gl.vertexAttribPointer(KVD.ATTRIBUTE_POSITION, 3, gl.FLOAT, false, 5 * 4, 0);
            gl.vertexAttribPointer(KVD.ATTRIBUTE_TEXTURE0, 2, gl.FLOAT, false, 5 * 4, 3 * 4);
            gl.enableVertexAttribArray(KVD.ATTRIBUTE_POSITION);
            gl.enableVertexAttribArray(KVD.ATTRIBUTE_TEXTURE0);
        }
        gl.bindBuffer(gl.ARRAY_BUFFER, null);
    }
    gl.bindVertexArray(null);

    // initialize cube projection matrix
    projectionMatrix_Cube = mat4.create();

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

    let aspectRatio = texWidth / texHeight;
    mat4.perspective(projectionMatrix_Sphere, Math.PI / 4, aspectRatio, 0.0, 100.0);

    aspectRatio = canvas.width / canvas.height;
    mat4.perspective(projectionMatrix_Cube, Math.PI / 4, aspectRatio, 0.0, 100.0);
}

function renderSphere() {
    // local variables
    var modelMatrix = mat4.create();
    
    // code
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
    gl.useProgram(lightingProgram);
    {
        // perform transformations
        lights[0].position = [
            0.0,
            10.0 * Math.sin(thetaLights * (Math.PI / 180.0)),
            10.0 * Math.cos(thetaLights * (Math.PI / 180.0)),
            1.0
        ];
        lights[1].position = [
            10.0 * Math.cos(thetaLights * (Math.PI / 180.0)),
            0.0,
            10.0 * Math.sin(thetaLights * (Math.PI / 180.0)),
            1.0
        ];
        lights[2].position = [
            10.0 * Math.cos(thetaLights * (Math.PI / 180.0)),
            10.0 * Math.sin(thetaLights * (Math.PI / 180.0)),
            0.0,
            1.0
        ];

        mat4.identity(modelMatrix);
        mat4.translate(modelMatrix, modelMatrix, [0.0, 0.0, -4.5]);

        // set up uniforms
        gl.uniformMatrix4fv(lightingUniforms.modelMatrix, false, modelMatrix);
        gl.uniformMatrix4fv(lightingUniforms.viewMatrix, false, mat4.create());
        gl.uniformMatrix4fv(lightingUniforms.projMatrix, false, projectionMatrix_Sphere);
        gl.uniform4fv(lightingUniforms.La[0], lights[0].ambient);
        gl.uniform4fv(lightingUniforms.Ld[0], lights[0].diffuse);
        gl.uniform4fv(lightingUniforms.Ls[0], lights[0].specular);
        gl.uniform4fv(lightingUniforms.lightPosition[0], lights[0].position);
        gl.uniform4fv(lightingUniforms.La[1], lights[1].ambient);
        gl.uniform4fv(lightingUniforms.Ld[1], lights[1].diffuse);
        gl.uniform4fv(lightingUniforms.Ls[1], lights[1].specular);
        gl.uniform4fv(lightingUniforms.lightPosition[1], lights[1].position);
        gl.uniform4fv(lightingUniforms.La[2], lights[2].ambient);
        gl.uniform4fv(lightingUniforms.Ld[2], lights[2].diffuse);
        gl.uniform4fv(lightingUniforms.Ls[2], lights[2].specular);
        gl.uniform4fv(lightingUniforms.lightPosition[2], lights[2].position);
        gl.uniform4fv(lightingUniforms.Ka, material.ambient);
        gl.uniform4fv(lightingUniforms.Kd, material.diffuse);
        gl.uniform4fv(lightingUniforms.Ks, material.specular);
        gl.uniform1f(lightingUniforms.materialShininess, material.shininess);
        gl.uniform4fv(lightingUniforms.viewerPosition, [0.0, 0.0, 4.5, 1.0]);
        gl.uniform1i(lightingUniforms.isLight, isLight ? 1 : 0);

        sphereObj.render();
    }
    gl.useProgram(null);

    updateSphere();
}

function render() {
    // code
    gl.clearColor(0.0, 0.0, 0.0, 1.0);
    gl.bindFramebuffer(gl.FRAMEBUFFER, fbo);
    gl.drawBuffers([gl.COLOR_ATTACHMENT0]);
    renderSphere();

    // render the cube with sphere texture
    gl.clearColor(1.0, 1.0, 1.0, 1.0);
    gl.bindFramebuffer(gl.FRAMEBUFFER, null);
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

    var translation = vec3.create();
    var modelViewMatrix = mat4.create();
    var modelViewProjectionMatrix = mat4.create();
    const theta_rad = (thetaCube * Math.PI) / 180.0;

    gl.useProgram(textureProgram);
    {
        // perform transformations
        mat4.identity(modelViewMatrix);
        mat4.identity(modelViewProjectionMatrix);

        vec3.set(translation, 0.0, 0.0, -6.0);
        mat4.translate(modelViewMatrix, modelViewMatrix, translation);
        mat4.rotateX(modelViewMatrix, modelViewMatrix, theta_rad);
        mat4.rotateY(modelViewMatrix, modelViewMatrix, theta_rad);
        mat4.rotateZ(modelViewMatrix, modelViewMatrix, theta_rad);

        mat4.multiply(modelViewProjectionMatrix, projectionMatrix_Cube, modelViewMatrix);

        // set up uniforms
        gl.uniformMatrix4fv(textureUniforms.mvpMatrix, false, modelViewProjectionMatrix);
        gl.uniform1i(textureUniforms.textureSampler, 0);

        gl.activeTexture(gl.TEXTURE0);
        gl.bindTexture(gl.TEXTURE_2D, texColor);

        // drawing commands
        gl.bindVertexArray(vaoCube);
            gl.drawArrays(gl.TRIANGLE_FAN, 0, 4);
            gl.drawArrays(gl.TRIANGLE_FAN, 4, 4);
            gl.drawArrays(gl.TRIANGLE_FAN, 8, 4);
            gl.drawArrays(gl.TRIANGLE_FAN, 12, 4);
            gl.drawArrays(gl.TRIANGLE_FAN, 16, 4);
            gl.drawArrays(gl.TRIANGLE_FAN, 20, 4);
        gl.bindVertexArray(null);
    }
    gl.useProgram(null);

    // swap buffers and maintain the rendering loop
    requestAnimationFrame(render, canvas);

    // update the frame
    update();
}

function updateSphere() {
    // code
    thetaLights += 1.0;
    if(thetaLights >= 360.0)
        thetaLights -= 360.0;
}

function update() {
    // code
    thetaCube += 1.0;
    if(thetaCube >= 360.0)
        thetaCube -= 360.0;
}

function uninitializeSphere() {
    // code
    if(sphereObj) {
        sphereObj.uninit();
    }

    if(lightingProgram != null)
    {
        let attachedShadersCount = gl.getProgramParameter(lightingProgram, gl.ATTACHED_SHADERS);
        if(attachedShadersCount > 0)
        {
            let attachedShadersList = gl.getAttachedShaders(lightingProgram);
            let reversedAttachedShadersList = attachedShadersList.reverse();

            for(let i = 0; i < attachedShadersCount; i++)
            {
                gl.detachShader(lightingProgram, reversedAttachedShadersList[i]);
                gl.deleteShader(reversedAttachedShadersList[i]);
                reversedAttachedShadersList[i] = null;
            }
        }
        
        console.log("uninitialize(): detached and deleted " + attachedShadersCount + " shader objects from lighting program\n");
        
        gl.deleteProgram(lightingProgram);
        lightingProgram = null;
    }

    if(textureProgram != null)
    {
        let attachedShadersCount = gl.getProgramParameter(textureProgram, gl.ATTACHED_SHADERS);
        if(attachedShadersCount > 0)
        {
            let attachedShadersList = gl.getAttachedShaders(textureProgram);
            let reversedAttachedShadersList = attachedShadersList.reverse();

            for(let i = 0; i < attachedShadersCount; i++)
            {
                gl.detachShader(textureProgram, reversedAttachedShadersList[i]);
                gl.deleteShader(reversedAttachedShadersList[i]);
                reversedAttachedShadersList[i] = null;
            }
        }
        
        console.log("uninitialize(): detached and deleted " + attachedShadersCount + " shader objects from texture program\n");
        
        gl.deleteProgram(textureProgram);
        textureProgram = null;
    }
}

function uninitialize() {
    // code
    if(isFullscreen)
        toggleFullscreen();

    if(vboCube) {
        gl.deleteBuffer(vboCube);
        vboCube = null;
    }

    if(vaoCube) {
        gl.deleteVertexArray(vaoCube);
        vaoCube = null;
    }

    if(texColor) {
        gl.deleteTexture(texColor);
        texColor = null;
    }

    if(rboDepth) {
        gl.deleteRenderbuffer(rboDepth);
        rboDepth = null;
    }

    if(fbo) {
        gl.deleteFramebuffer(fbo);
        fbo = null;
    }

    uninitializeSphere();
}
