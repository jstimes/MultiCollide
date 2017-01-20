function drawVisualizations() {
	drawGraph1();
	drawGraph2();
}
  
 var timeout = 80;
  
var data = null;
var graph = null;
var t = 0;

function grow() {
	t++;
	data.add({x: t, y: t, z: t});
	graph.setData(data);
	
	if(t < 200){
		setTimeout(grow, timeout);
	}
}

function drawGraph1() {
	
	drawVisualization();

	// Called when the Visualization API is loaded.
	function drawVisualization() {
	  // Create and populate a data table.
	  data = new vis.DataSet();

	  var tMax = 200;

			
	  data.add({x:0,y:0,z:0});

	  // specify options
	  var options = {
		width:  '220px',
		height: '220px',
		style: 'line',
		showPerspective: true,
		showGrid: true,
		showShadow: false,
		keepAspectRatio: true,
		verticalRatio: 0.5,
		
		xMax: 200,
		yMax: 200, 
		zMax: 200, 
		xMin: 0, 
		yMin: 0, 
		zMin: 0,
	  };

	  // create our graph
	  var container = document.getElementById('graph1');
	  graph = new vis.Graph3d(container, data, options);
	  
	  grow();

	}
}

var data2 = null;
var graph2 = null;
var t2 = 0;

function grow2() {
	t2++;
	data2.add({x: t2, y: Math.sin(t2), z: t});
	graph2.setData(data2);
	
	if(t2 < 200){
		setTimeout(grow2, timeout);
	}
}

function drawGraph2() {
	
	drawVisualization();

	// Called when the Visualization API is loaded.
	function drawVisualization() {
	  // Create and populate a data table.
	  data2 = new vis.DataSet();

	  var tMax = 200;

			
	  data2.add({x:0,y:0,z:0});

	  // specify options
	  var options = {
		width:  '220px',
		height: '220px',
		style: 'line',
		showPerspective: true,
		showGrid: true,
		showShadow: false,
		keepAspectRatio: true,
		verticalRatio: 0.5,
		
		xMax: 200,
		yMax: 200, 
		zMax: 200, 
		xMin: 0, 
		yMin: 0, 
		zMin: 0,
	  };

	  // create our graph
	  var container = document.getElementById('graph2');
	  graph2 = new vis.Graph3d(container, data2, options);
	  
	  grow2();

	}
}