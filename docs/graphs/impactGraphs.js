//Controls speed of graph growth.
// ms between plotting graph points
var timeout = 80;

var ig;
var vg;

function drawImpulse(impulsePtsArr) {
	ig = new ImpulseGraph(impulsePtsArr);
}

function drawVelocity(velocityPtsArr){
	vg = new VelocityGraph(velocityPtsArr);
}

function stopVisualizations(){
	if(ig)
		ig.stop();
	
	if(vg)
		vg.stop();
}

function ImpulseGraph(impulsePtsArr) {
	
	this.pts = impulsePtsArr;
	this.graph = null;
	this.data = null;
	this.curPt = 0;
	this.shouldStop = false;
	
	this.drawVisualization();
}

ImpulseGraph.prototype.stop = function() {
	this.shouldStop = true;
}

ImpulseGraph.prototype.grow = function() {
	if(this.shouldStop){
		return;
	}
	
	var pt = {x: this.pts[this.curPt], y: this.pts[this.urPt+1], z: this.pts[this.curPt+2]};
		
	this.data.add(pt);
	this.curPt += 3;
	
	this.graph.setData(this.data);
	
	var me = this;
	
	if(!this.shouldStop && this.curPt < this.pts.length){
		setTimeout(this.grow.bind(me), timeout);
	}
}

ImpulseGraph.prototype.drawVisualization = function() {
	// Create and populate a data table.
	  this.data = new vis.DataSet();

	  this.data.add({x: this.pts[this.curPt], y: this.pts[this.curPt+1], z: this.pts[this.curPt+2]});
	  this.curPt += 3;
	  
	  var max = 0;
	  var min = 0;
	  
	  for(var p=0; p<this.pts.length; p++){
		if(this.pts[p] > max){
			max = this.pts[p];
		}
		if(this.pts[p] < min){
			min = this.pts[p];
		}
	  }
	  max += .1;
	  min -= .1;

	  // specify options
	  var options = {
		width:  '165px',
		height: '165px',
		style: 'line',
		showPerspective: true,
		showGrid: true,
		showShadow: false,
		keepAspectRatio: true,
		verticalRatio: 0.5,
		
		xMax: max,
		yMax: max, 
		zMax: max, 
		xMin: min, 
		yMin: min, 
		zMin: min,
		
		dataColor: {fill: '#FF0000', stroke: '#FF0000', strokeWidth: 1}
	  };

	  // create our graph
	  var container = document.getElementById('graph1');
	  this.graph = new vis.Graph3d(container, this.data, options);
	  
	  this.grow();
}

function VelocityGraph(velocityPtsArr) {
	
	this.pts = velocityPtsArr;
	this.data = null;
	this.layout = null;
	this.curPt = 0;
	this.shouldStop = false;
	
	this.draw();
}
	
	
VelocityGraph.prototype.draw = function () {
		
	this.data = {
		x: [],
		y: [],
		line: {shape: 'spline'},
		mode: 'lines'
	};

	this.layout = { 
		//title: "Sliding Velocity Curve (Hodograph)",
		width:  150,
		height: 150,
		margin: {
			l: 0,
			r: 0,
			b: 0,
			t: 0,
			pad: 4
		  },
	};
	
	this.grow();
}

VelocityGraph.prototype.stop = function() {
	this.shouldStop = true;
	Plotly.purge('graph2');
}

VelocityGraph.prototype.grow = function() {
	if(this.shouldStop){
		return;
	}
	
	this.data.x.push(this.pts[this.curPt]);
	this.data.y.push(this.pts[this.curPt+1]);
	this.curPt += 3;
	
	//TODO add data with mode 'scatter' for points (EOC, EOR)
	var plots = [this.data]; 
	Plotly.newPlot('graph2', plots, this.layout, {displayModeBar: false});
	
	var me = this;
	
	if(!this.shouldStop && this.curPt < this.pts.length){
		setTimeout(this.grow.bind(me), timeout);
	}
}