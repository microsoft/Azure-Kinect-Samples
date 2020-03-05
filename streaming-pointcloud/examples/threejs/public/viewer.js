const socket = io();
let sceneReady = false;


socket.on("points", function(data) {
    parsePointcloudFrame(data);
});

let scene;
let renderer;
let camera;

let pointsGeometry;
let pointsMaterial;
let pointsMesh;

let angle = 0;
let lastFrameTime = 0;


function parsePointcloudFrame(data) {
    
    if(!sceneReady) {
        // Ignore frames that arrive before the THREE.js renderer and scene are initialized.
        return;
    }
    
    let timeStamp   = new Uint32Array(data, 0, 1)[0];
    let pointCount  = new Uint32Array(data, 8, 1)[0];
    let flags       = new Uint8Array(data, 12, 1)[0];

    let payload = data.slice(13);

    let hasColorData = (flags & 0x1) != 0;
    
    
    // X,Y,Z points are stored as 16-bit integers to reduce bandwidth. These
    // need to be converted to 32-bit floats for use with the THREE.js geometry.
    let positionData = new Int16Array(payload, 0, pointCount * 3);
    let positions    = new Float32Array(pointCount * 3);
    
    // Convert coordinates from ints to floats.    
    for(var i = 0; i < pointCount * 3; i++) {
        positions[i] = positionData[i] * 0.01;
    }

    // Update the "position" attribute using the new positions from the latest data frame.
    pointsGeometry.setAttribute("position", new THREE.Float32BufferAttribute( positions, 3 ) );

    // If the frame contains color data, update the "color" attribute too.
    if(hasColorData) {

        // Color data is stored after all of the position data, compute the byte offset:
        let colorDataOffset = (pointCount * 3) * 2; // Each point has three 16-bit values for position data

        let colorPayload = payload.slice(colorDataOffset);
        let colorData = new Uint8Array(colorPayload, 0, pointCount * 3);
        let colors = new Float32Array(pointCount * 3);
        
        // convert from bytes to floating point values for RGB data
        for(var i = 0; i < pointCount * 3; i++) {
            colors[i] = colorData[i] / 255;
        }

        pointsGeometry.setAttribute( "color", new THREE.Float32BufferAttribute( colors, 3 ) );
    }

    document.getElementById("stats").innerHTML = `Points: ${pointCount}<br>Has Color: ${hasColorData}`;
}


function InitializeScene() {
    
    camera = new THREE.PerspectiveCamera(45, window.innerWidth / window.innerHeight, 0.1, 1000);
    
    scene = new THREE.Scene();
    renderer = new THREE.WebGLRenderer({
        "canvas" : document.getElementById("viewport")
    });
    
    pointsGeometry = new THREE.BufferGeometry();
    pointsMaterial = new THREE.PointsMaterial( { 
        color: 0x888888,
        size: 0.25,
        vertexColors: THREE.VertexColors

    } );
    
    pointsMesh = new THREE.Points(pointsGeometry, pointsMaterial);
    let s = 1;
    pointsMesh.scale.set(s,s,s);
    pointsMesh.position.y = -5;

    scene.add(pointsMesh);

    camera.position.set(0, 10, 50);
    camera.lookAt(scene.position);

    resizeViewport();
    render();

    sceneReady = true;
}


function resizeViewport() { 
    let height = window.innerHeight;
    let width = window.innerWidth;

    let aspectRatio = width / height;
    
    renderer.setSize(width,height);
    camera.aspect = aspectRatio;
    camera.updateProjectionMatrix();
}


function render() {

    requestAnimationFrame(render);
    
    let deltaTime = Date.now() - lastFrameTime;
    
    if(lastFrameTime == 0) {
        deltaTime = 0;
    }

    renderer.render(scene, camera);

    pointsMesh.rotation.y += deltaTime * 0.0001;
    lastFrameTime = Date.now();

}

window.addEventListener("resize", resizeViewport);
InitializeScene();