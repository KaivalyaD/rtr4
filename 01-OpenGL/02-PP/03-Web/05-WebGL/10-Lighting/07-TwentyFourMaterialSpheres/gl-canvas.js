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
    La : null,
    Ld : null,
    Ls : null,
    lightPosition: null,
    Ka : null,
    Kd : null,
    Ks : null,
    materialShininess : null,
    viewerPosition : null,
    isLight : null
};

// lighting
var light = {
    ambient : null,
    diffuse : null,
    specular : null,
    position : null,
};
var material = {
    ambient : null,
    diffuse : null,
    specular : null,
    shininess: null
};

// for the App
var sphereObj = null;
var perspectiveProjectionMatrix = null;
var isLight = false;
var axis = 0;
var theta = 0.0;

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

    case 88:  // X
    case 120: // x
        axis = 1;
        break;

    case 89:  // Y
    case 121: // y
        axis = 2;
        break;

    case 90:  // Z
    case 122: // z
        axis = 3;
        break;
    
    default:
        axis = 0;
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
    
    uniform vec4 lightPosition;  // in eye coordinates
    uniform vec4 viewerPosition; // in eye coordinates

    uniform int isLight;

    out vec3 a_normal_out;
    out vec3 a_lightDirection_out;
    out vec3 a_viewDirection_out;

    void main(void)
    {
        mat4 mvMatrix = viewMatrix * modelMatrix;
        vec4 eyeCoordinate = mvMatrix * a_position;
        if(isLight != 0) {
            mat3 normalMatrix = mat3(transpose(inverse(mvMatrix)));
            a_normal_out = normalMatrix * a_normal;
            a_lightDirection_out = lightPosition.xyz - eyeCoordinate.xyz;
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

    in vec3 a_normal_out;
    in vec3 a_lightDirection_out;
    in vec3 a_viewDirection_out;

    uniform vec4 La;
    uniform vec4 Ld;
    uniform vec4 Ls;

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
            vec3 lightDirection = normalize(a_lightDirection_out);
            vec3 viewDirection = normalize(a_viewDirection_out);            
            vec3 reflectedDirection = reflect(-lightDirection, normal);

            vec4 ambient = La * Ka;
            vec4 diffuse = Ld * Kd * max(dot(normal, lightDirection), 0.0);
            vec4 specular = Ls * Ks * pow(max(dot(reflectedDirection, viewDirection), 0.0), materialShininess);

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
    uniforms.La = gl.getUniformLocation(shaderProgramObject, "La");
    uniforms.Ld = gl.getUniformLocation(shaderProgramObject, "Ld");
    uniforms.Ls = gl.getUniformLocation(shaderProgramObject, "Ls");
    uniforms.lightPosition = gl.getUniformLocation(shaderProgramObject, "lightPosition");
    uniforms.Ka = gl.getUniformLocation(shaderProgramObject, "Ka");
    uniforms.Kd = gl.getUniformLocation(shaderProgramObject, "Kd");
    uniforms.Ks = gl.getUniformLocation(shaderProgramObject, "Ks");
    uniforms.materialShininess = gl.getUniformLocation(shaderProgramObject, "materialShininess");
    uniforms.viewerPosition = gl.getUniformLocation(shaderProgramObject, "viewerPosition");
    uniforms.isLight = gl.getUniformLocation(shaderProgramObject, "isLight");

    // generate a sphere & put geometry data into rendering pipeline
    sphereObj = new sphere(25, 25);
    sphereObj.init(KVD.ATTRIBUTE_POSITION, KVD.ATTRIBUTE_NORMAL);

    // create and initialize perspective projection matrix
    perspectiveProjectionMatrix = mat4.create();
    mat4.identity(perspectiveProjectionMatrix);

    // setup light params
    light.ambient = [0.1, 0.1, 0.1, 1.0];
    light.diffuse = [1.0, 1.0, 1.0, 1.0];
    light.specular = [1.0, 1.0, 1.0, 1.0];
    light.position = [0.0, 0.0, 10.0, 1.0];

    // setup material params
    material.ambient = [0.0, 0.0, 0.0, 1.0];
    material.diffuse = [0.5, 0.2, 0.7, 1.0];
    material.specular = [0.7, 0.7, 0.7, 1.0];
    material.shininess = 128.0;

    // set clear color
    gl.clearColor(0.15, 0.15, 0.15, 1.0);

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

    var aspectRatio = (canvas.width / 4.0) / (canvas.height / 6.0);
    mat4.perspective(perspectiveProjectionMatrix, Math.PI / 4.0, aspectRatio, 0.0, 100.0);
}

function render() {
    // code
    gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

    // update light positions
    switch(axis) {
        case 0:
            light.position = [0.0, 0.0, 10.0, 1.0];
            break;

        case 1:
            light.position = [
                0.0,
                10.0 * Math.cos(theta * (Math.PI / 180.0)),
                10.0 * Math.sin(theta * (Math.PI / 180.0)),
                1.0
            ];
            break;

        case 2:
            light.position = [
                10.0 * Math.sin(theta * (Math.PI / 180.0)),
                0.0,
                10.0 * Math.cos(theta * (Math.PI / 180.0)),
                1.0
            ];
            break;

        case 3:
            light.position = [
                10.0 * Math.cos(theta * (Math.PI / 180.0)),
                10.0 * Math.sin(theta * (Math.PI / 180.0)),
                0.0,
                1.0
            ];
            break;
        
        default:
            break;
    }

    // render
    draw24Spheres();

    // swap buffers and maintain the rendering loop
    requestAnimationFrame(render, canvas);

    // update the frame
    update();
}

function update() {
    // code
    theta += 1.0;
    if(theta > 360.0)
        theta -= 360.0;
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

function draw24Spheres() {
    var modelMatrix = mat4.create();
    gl.useProgram(shaderProgramObject)
    {
        // perform transformations
        mat4.identity(modelMatrix);
        mat4.translate(modelMatrix, modelMatrix, [0.0, 0.0, -4.5]);

        // setup independent uniforms
        gl.uniformMatrix4fv(uniforms.modelMatrix, false, modelMatrix);
        gl.uniformMatrix4fv(uniforms.viewMatrix, false, mat4.create());
        gl.uniformMatrix4fv(uniforms.projMatrix, false, perspectiveProjectionMatrix);

        if(light.isOn) {
            // setup dependent uniforms
            gl.uniform4fv(uniforms.La, light.ambient);
            gl.uniform4fv(uniforms.Ld, light.diffuse);
            gl.uniform4fv(uniforms.Ls, light.specular);
            gl.uniform4fv(uniforms.lightPosition, light.position);
            gl.uniform4fv(uniforms.viewerPosition, [0.0, 0.0, 4.5, 1.0]);
            gl.uniform1i(uniforms.isLight, 1);

            // draw
            // [1, 1] material: emerald
            gl.viewport(0, 5 * (canvas.height / 6), canvas.width / 4, canvas.height / 6);
            gl.uniform4fv(uniforms.Ka, [0.0215, 0.1745, 0.0215, 1.0]);
            gl.uniform4fv(uniforms.Kd, [0.07568, 0.61424, 0.07568, 1.0]);
            gl.uniform4fv(uniforms.Ks, [0.633, 0.727811, 0.633, 1.0]);
            gl.uniform1f(uniforms.materialShininess, 0.6 * 128.0);
            sphereObj.render();

            // [2, 1] material: jade
            gl.viewport(0, 4 * (canvas.height / 6), canvas.width / 4, canvas.height / 6);
            gl.uniform4fv(uniforms.Ka, [0.135, 0.2225, 0.1575, 1.0]);
            gl.uniform4fv(uniforms.Kd, [0.54, 0.89, 0.63, 1.0]);
            gl.uniform4fv(uniforms.Ks, [0.316228, 0.316228, 0.316228, 1.0]);
            gl.uniform1f(uniforms.materialShininess, 0.1 * 128.0);
            sphereObj.render();

            // [3, 1] material: obsidian
            gl.viewport(0, 3 * (canvas.height / 6), canvas.width / 4, canvas.height / 6);
            gl.uniform4fv(uniforms.Ka, [0.05375, 0.05, 0.06625, 1.0]);
            gl.uniform4fv(uniforms.Kd, [0.18275, 0.17, 0.22525, 1.0]);
            gl.uniform4fv(uniforms.Ks, [0.332741, 0.328634, 0.346435, 1.0]);
            gl.uniform1f(uniforms.materialShininess, 0.3 * 128.0);
            sphereObj.render();

            // [4, 1] material: pearl
            gl.viewport(0, 2 * (canvas.height / 6), canvas.width / 4, canvas.height / 6);
            gl.uniform4fv(uniforms.Ka, [0.25, 0.20725, 0.20725, 1.0]);
            gl.uniform4fv(uniforms.Kd, [1.0, 0.829, 0.829, 1.0]);
            gl.uniform4fv(uniforms.Ks, [0.296648, 0.296648, 0.296648, 1.0]);
            gl.uniform1f(uniforms.materialShininess, 0.3 * 128.0);
            sphereObj.render();

            // [5, 1] material: ruby
            gl.viewport(0, (canvas.height / 6), canvas.width / 4, canvas.height / 6);
            gl.uniform4fv(uniforms.Ka, [0.1745, 0.01175, 0.01175, 1.0]);
            gl.uniform4fv(uniforms.Kd, [0.61424, 0.04136, 0.04136, 1.0]);
            gl.uniform4fv(uniforms.Ks, [0.727811, 0.727811, 0.727811, 1.0]);
            gl.uniform1f(uniforms.materialShininess, 0.6 * 128.0);
            sphereObj.render();

            // [6, 1] material: turquoise
            gl.viewport(0, 0, canvas.width / 4, canvas.height / 6);
            gl.uniform4fv(uniforms.Ka, [0.1, 0.18725, 0.1745, 1.0]);
            gl.uniform4fv(uniforms.Kd, [0.396, 0.74151, 0.69102, 1.0]);
            gl.uniform4fv(uniforms.Ks, [0.297254, 0.30829, 0.306678, 1.0]);
            gl.uniform1f(uniforms.materialShininess, 0.1 * 128.0);
            sphereObj.render();

            // [1, 2] material: brass
            gl.viewport(canvas.width / 4, 5 * (canvas.height / 6), canvas.width / 4, canvas.height / 6);
            gl.uniform4fv(uniforms.Ka, [0.329412, 0.223529, 0.027451, 1.0]);
            gl.uniform4fv(uniforms.Kd, [0.780392, 0.568627, 0.113725, 1.0]);
            gl.uniform4fv(uniforms.Ks, [0.992157, 0.941176, 0.807843, 1.0]);
            gl.uniform1f(uniforms.materialShininess, 0.21794872 * 128.0);
            sphereObj.render();

            // [2, 2] material: bronze
            gl.viewport(canvas.width / 4, 4 * (canvas.height / 6), canvas.width / 4, canvas.height / 6);
            gl.uniform4fv(uniforms.Ka, [0.2125, 0.1275, 0.054, 1.0]);
            gl.uniform4fv(uniforms.Kd, [0.714, 0.4284, 0.18144, 1.0]);
            gl.uniform4fv(uniforms.Ks, [0.393548, 0.271906, 0.166721, 1.0]);
            gl.uniform1f(uniforms.materialShininess, 0.2 * 128.0);
            sphereObj.render();

            // [3, 2] material: chrome
            gl.viewport(canvas.width / 4, 3 * (canvas.height / 6), canvas.width / 4, canvas.height / 6);
            gl.uniform4fv(uniforms.Ka, [0.25, 0.25, 0.25, 1.0]);
            gl.uniform4fv(uniforms.Kd, [0.4, 0.4, 0.4, 1.0]);
            gl.uniform4fv(uniforms.Ks, [0.774597, 0.774597, 0.774597, 1.0]);
            gl.uniform1f(uniforms.materialShininess, 0.6 * 128.0);
            sphereObj.render();

            // [4, 2] material: copper
            gl.viewport(canvas.width / 4, 2 * (canvas.height / 6), canvas.width / 4, canvas.height / 6);
            gl.uniform4fv(uniforms.Ka, [0.19125, 0.0735, 0.0225, 1.0]);
            gl.uniform4fv(uniforms.Kd, [0.7038, 0.27048, 0.0828, 1.0]);
            gl.uniform4fv(uniforms.Ks, [0.256777, 0.137622, 0.086014, 1.0]);
            gl.uniform1f(uniforms.materialShininess, 0.1 * 128.0);
            sphereObj.render();

            // [5, 2] material: gold
            gl.viewport(canvas.width / 4, canvas.height / 6, canvas.width / 4, canvas.height / 6);
            gl.uniform4fv(uniforms.Ka, [0.24725, 0.1995, 0.0745, 1.0]);
            gl.uniform4fv(uniforms.Kd, [0.75164, 0.60648, 0.22648, 1.0]);
            gl.uniform4fv(uniforms.Ks, [0.628281, 0.555802, 0.366065, 1.0]);
            gl.uniform1f(uniforms.materialShininess, 0.4 * 128.0);
            sphereObj.render();

            // [6, 2] material: silver
            gl.viewport(canvas.width / 4, 0, canvas.width / 4, canvas.height / 6);
            gl.uniform4fv(uniforms.Ka, [0.19225, 0.19225, 0.19225, 1.0]);
            gl.uniform4fv(uniforms.Kd, [0.50754, 0.50754, 0.50754, 1.0]);
            gl.uniform4fv(uniforms.Ks, [0.508273, 0.508273, 0.508273, 1.0]);
            gl.uniform1f(uniforms.materialShininess, 0.4 * 128.0);
            sphereObj.render();

            // [1, 3] material: black plastic
            gl.viewport(2 * (canvas.width / 4), 5 * (canvas.height / 6), canvas.width / 4, canvas.height / 6);
            gl.uniform4fv(uniforms.Ka, [0.0, 0.0, 0.0, 1.0]);
            gl.uniform4fv(uniforms.Kd, [0.01, 0.01, 0.01, 1.0]);
            gl.uniform4fv(uniforms.Ks, [0.5, 0.5, 0.5, 1.0]);
            gl.uniform1f(uniforms.materialShininess, 0.25 * 128.0);
            sphereObj.render();

            // [2, 3] material: cyan plastic
            gl.viewport(2 * (canvas.width / 4), 4 * (canvas.height / 6), canvas.width / 4, canvas.height / 6);
            gl.uniform4fv(uniforms.Ka, [0.0, 0.1, 0.0, 1.0]);
            gl.uniform4fv(uniforms.Kd, [0.0, 0.50980392, 0.50980392, 1.0]);
            gl.uniform4fv(uniforms.Ks, [0.50196078, 0.50196078, 0.50196078, 1.0]);
            gl.uniform1f(uniforms.materialShininess, 0.25 * 128.0);
            sphereObj.render();

            // [3, 3] material: green plastic
            gl.viewport(2 * (canvas.width / 4), 3 * (canvas.height / 6), canvas.width / 4, canvas.height / 6);
            gl.uniform4fv(uniforms.Ka, [0.0, 0.0, 0.0, 1.0]);
            gl.uniform4fv(uniforms.Kd, [0.1, 0.35, 0.1, 1.0]);
            gl.uniform4fv(uniforms.Ks, [0.45, 0.55, 0.45, 1.0]);
            gl.uniform1f(uniforms.materialShininess, 0.25 * 128.0);
            sphereObj.render();

            // [4, 3] material: red plastic
            gl.viewport(2 * (canvas.width / 4), 2 * (canvas.height / 6), canvas.width / 4, canvas.height / 6);
            gl.uniform4fv(uniforms.Ka, [0.0, 0.0, 0.0, 1.0]);
            gl.uniform4fv(uniforms.Kd, [0.5, 0.0, 0.0, 1.0]);
            gl.uniform4fv(uniforms.Ks, [0.7, 0.6, 0.6, 1.0]);
            gl.uniform1f(uniforms.materialShininess, 0.25 * 128.0);
            sphereObj.render();

            // [5, 3] material: white plastic
            gl.viewport(2 * (canvas.width / 4), canvas.height / 6, canvas.width / 4, canvas.height / 6);
            gl.uniform4fv(uniforms.Ka, [0.0, 0.0, 0.0, 1.0]);
            gl.uniform4fv(uniforms.Kd, [0.55, 0.55, 0.55, 1.0]);
            gl.uniform4fv(uniforms.Ks, [0.7, 0.7, 0.7, 1.0]);
            gl.uniform1f(uniforms.materialShininess, 0.25 * 128.0);
            sphereObj.render();

            // [6, 3] material: yellow plastic
            gl.viewport(2 * (canvas.width / 4), 0, canvas.width / 4, canvas.height / 6);
            gl.uniform4fv(uniforms.Ka, [0.0, 0.0, 0.0, 1.0]);
            gl.uniform4fv(uniforms.Kd, [0.5, 0.5, 0.0, 1.0]);
            gl.uniform4fv(uniforms.Ks, [0.6, 0.6, 0.5, 1.0]);
            gl.uniform1f(uniforms.materialShininess, 0.25 * 128.0);
            sphereObj.render();

            // [1, 4] material: black rubber
            gl.viewport(3 * (canvas.width / 4), 5 * (canvas.height / 6), canvas.width / 4, canvas.height / 6);
            gl.uniform4fv(uniforms.Ka, [0.02, 0.02, 0.02, 1.0]);
            gl.uniform4fv(uniforms.Kd, [0.01, 0.01, 0.01, 1.0]);
            gl.uniform4fv(uniforms.Ks, [0.4, 0.4, 0.4, 1.0]);
            gl.uniform1f(uniforms.materialShininess, 0.078125 * 128.0);
            sphereObj.render();

            // [2, 4] material: cyan rubber
            gl.viewport(3 * (canvas.width / 4), 4 * (canvas.height / 6), canvas.width / 4, canvas.height / 6);
            gl.uniform4fv(uniforms.Ka, [0.0, 0.05, 0.05, 1.0]);
            gl.uniform4fv(uniforms.Kd, [0.4, 0.5, 0.5, 1.0]);
            gl.uniform4fv(uniforms.Ks, [0.04, 0.7, 0.7, 1.0]);
            gl.uniform1f(uniforms.materialShininess, 0.078125 * 128.0);
            sphereObj.render();

            // [3, 4] material: green rubber
            gl.viewport(3 * (canvas.width / 4), 3 * (canvas.height / 6), canvas.width / 4, canvas.height / 6);
            gl.uniform4fv(uniforms.Ka, [0.0, 0.05, 0.05, 1.0]);
            gl.uniform4fv(uniforms.Kd, [0.4, 0.5, 0.4, 1.0]);
            gl.uniform4fv(uniforms.Ks, [0.04, 0.7, 0.04, 1.0]);
            gl.uniform1f(uniforms.materialShininess, 0.078125 * 128.0);
            sphereObj.render();

            // [4, 4] material: red rubber
            gl.viewport(3 * (canvas.width / 4), 2 * (canvas.height / 6), canvas.width / 4, canvas.height / 6);
            gl.uniform4fv(uniforms.Ka, [0.05, 0.0, 0.0, 1.0]);
            gl.uniform4fv(uniforms.Kd, [0.5, 0.4, 0.4, 1.0]);
            gl.uniform4fv(uniforms.Ks, [0.7, 0.04, 0.04, 1.0]);
            gl.uniform1f(uniforms.materialShininess, 0.078125 * 128.0);
            sphereObj.render();

            // [5, 4] material: white rubber
            gl.viewport(3 * (canvas.width / 4), canvas.height / 6, canvas.width / 4, canvas.height / 6);
            gl.uniform4fv(uniforms.Ka, [0.05, 0.05, 0.05, 1.0]);
            gl.uniform4fv(uniforms.Kd, [0.5, 0.5, 0.5, 1.0]);
            gl.uniform4fv(uniforms.Ks, [0.7, 0.7, 0.7, 1.0]);
            gl.uniform1f(uniforms.materialShininess, 0.078125 * 128.0);
            sphereObj.render();

            // [6, 4] material: yellow rubber
            gl.viewport(3 * (canvas.width / 4), 0, canvas.width / 4, canvas.height / 6);
            gl.uniform4fv(uniforms.Ka, [0.05, 0.05, 0.0, 1.0]);
            gl.uniform4fv(uniforms.Kd, [0.5, 0.5, 0.4, 1.0]);
            gl.uniform4fv(uniforms.Ks, [0.7, 0.7, 0.04, 1.0]);
            gl.uniform1f(uniforms.materialShininess, 0.078125 * 128.0);
            sphereObj.render();
        }
        else {
            // setup dependent uniforms
            gl.uniform1i(uniforms.isLight, 0);

            // draw
            for(let x = 0; x < 4; x++)
            {
                for(let y = 1; y <= 6; y++)
                {
                    gl.viewport(x * (canvas.width / 4), canvas.height - (y * canvas.height / 6), canvas.width / 4, canvas.height / 6);
                    sphereObj.render();
                }
            }
        }
    }
    gl.useProgram(null);
}
