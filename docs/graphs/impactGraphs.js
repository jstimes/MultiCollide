//Controls speed of graph growth.
// ms between plotting graph points
var timeout = 80;

var ig;
var vg;

function drawImpulse(impulsePtsArr, endOfCompression, endOfSliding) {
	ig = new ImpulseGraph(impulsePtsArr, endOfCompression, endOfSliding);
}

function drawVelocity(velocityPtsArr, endOfCompression, endOfSliding, graphId){
	//vg = new VelocityGraph(velocityPtsArr);
    if(!graphId){
        graphId = "#graph2";
    }
	var ctx = $(graphId);
	
	var max = 0;
    var min = 0;
  
	//var xPts = [];
	//var yPts = [];
	var all_data = [];
	var compressionPt = [];
	var slidingPt = [];
	//alert("Num vel. pts: " + velocityPtsArr.length);
    for(var p=0; p<velocityPtsArr.length; p +=3){
		//xPts.push(velocityPtsArr[p]);
		//yPts.push(velocityPtsArr[p+1]);
		
		var pt = {x: velocityPtsArr[p], y: velocityPtsArr[p+1]};
		all_data.push(pt);
		
		if(p/3 == endOfCompression){
			compressionPt.push(pt);
		}
		
		if(p/3 == endOfSliding){
			slidingPt.push(pt);
		}
		
		// if(this.pts[p] > max){
			// max = this.pts[p];
		// }
		// if(this.pts[p] < min){
			// min = this.pts[p];
		// }
		
		// if(this.pts[p+1] > max){
			// max = this.pts[p+1];
		// }
		// if(this.pts[p+1] < min){
			// min = this.pts[p+1];
		// }
    }
	
    //max += Math.abs(max * .1);
    //min -= Math.abs(min * .1);
	
	var cur = 0;
	var curData = [ all_data[cur++] ];
	
	var myLineChart = new Chart(ctx, {
		type: 'line',
		data: {
			datasets: [{
				//label: 'Sliding Velocity Curve (Hodograph)',
				data: curData,
				pointBackgroundColor: "#B00000",
                showLine: true,
                borderColor: "#B00000",
                pointRadius: .1,
			},
			{
				data: slidingPt,
				pointBackgroundColor: "#00FF00",
                pointRadius: 2.0
			},
			{
				data: compressionPt,
				pointBackgroundColor: "#0000FF",
                pointRadius: 2.0
			}]
		},
		
		options: {
			//animation: {
			//	easing: 'linear',
			//	onComplete: function () {
			//		
			//	},
			//},
			//easing: 'linear',
			scales: {
				xAxes: [{
					type: 'linear',
					position: 'bottom'
				}]
			},
			legend: {
				display: false 
			}
		}
	});
	
	var grow = function() {
		//console.log("On complete called");
		if(cur < all_data.length){
			//console.log("Ading pt");
			myLineChart.data.datasets[0].data.push(all_data[cur++]);
			myLineChart.reset();
			myLineChart.update();
			setTimeout(grow, 100);
		}
	};
	setTimeout(grow, 100);
}

function stopVisualizations(){
	if(ig)
		ig.stop();
	
	if(vg)
		vg.stop();
}

function ImpulseGraph(impulsePtsArr, endOfCompression, endOfSliding) {
	
	this.endOfCompression = endOfCompression;
	this.endOfSliding = endOfSliding;
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
	
	var pt = {x: this.pts[this.curPt], y: this.pts[this.curPt+1], z: this.pts[this.curPt+2]};
	
	if(this.curPt / 3 == this.endOfCompression){
		pt.style = 100;
		console.log("end of compression pt");
	}
	if(this.curPt / 3 == this.endOfSliding){
		pt.style = 50;
		console.log("end of sliding pt");
	}
		
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
	  /*
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
	  max += Math.abs(max * .1);
	  min -= Math.abs(min * .1);
      */
      var xMin = 100;
      var yMin = 100;
      var zMin = 100;
      var xMax = -100;
      var yMax = -100;
      var zMax = -100;
      for (var p=0; p<this.pts.length; p = p + 3){
          var x = this.pts[p];
          var y = this.pts[p+1];
          var z = this.pts[p+2];
          
          if(x > xMax) xMax = x;
          if(x < xMin) xMin = x;
          if(y > yMax) yMax = y;
          if(y < yMin) yMin = y;
          if(z > zMax) zMax = z;
          if(z < zMin) zMin = z;
      }
      
      var xRange = xMax - xMin;
      var yRange = yMax - yMin;
      var zRange = zMax - zMin;
      
      if(xRange > yRange && xRange > zRange){
          if(yRange > zRange){
              var additional = yRange - zRange;
              zMax = zMax + (additional / 2.0);
              zMin = zMin - (additional / 2.0);
              zRange = zRange + additional;
              console.log('z smallest, x biggest');
          }
          else {
              var additional = zRange - yRange;
              yMax = yMax + (additional / 2.0);
              yMin = yMin - (additional / 2.0);
              yRange = yRange + additional;
              console.log('y smallest, x biggest');
          }
      }
      else if (yRange > xRange && yRange > zRange){
          if(xRange > zRange){
              var additional = xRange - zRange;
              zMax = zMax + (additional / 2.0);
              zMin = zMin - (additional / 2.0);
              zRange = zRange + additional;
              console.log('z smallest, y biggest');
          }
          else {
              var additional = zRange - xRange;
              xMax = xMax + (additional / 2.0);
              xMin = xMin - (additional / 2.0);
              xRange = xRange + additional;
              console.log('x smallest, y biggest');
          }
      }
      else{
          if(xRange > yRange){
              var additional = xRange - yRange;
              yMax = yMax + (additional / 2.0);
              yMin = yMin - (additional / 2.0);
              yRange = yRange + additional;
              console.log('y smallest, z biggest');
          }
          else {
              var additional = yRange - xRange;
              xMax = xMax + (additional / 2.0);
              xMin = xMin - (additional / 2.0);
              xRange = xRange + additional;
              console.log('x smallest, z biggest');
          }
      }
      
      var steps = 4;
      var xStep = xRange / steps;
      var yStep = yRange / steps;
      var zStep = zRange / steps;
      
      /*
      var step = .5;
      var range = max - min;
      console.log("Range: " + range);
      if(range < 1.0){
          step = .25;
      }
      else if (range > 3.0){
          step = 1.0;
      }
      if(range < step){
          step = range / 4.0;
          step = +step.toFixed(2);
      }
      console.log("Step: " + step);
      */

	  // specify options
	  var options = {
		width:  '230px',
		height: '230px',
		style: 'line',
		showPerspective: true,
		showGrid: true,
		showShadow: false,
		keepAspectRatio: true,
		verticalRatio: 0.5,
		
		xMax: xMax,
		yMax: yMax, 
		zMax: zMax, 
		xMin: xMin, 
		yMin: yMin, 
		zMin: zMin,
        
        xStep: Math.round(xStep*1000)/1000,
        yStep: Math.round(yStep*1000)/1000, 
        zStep: Math.round(zStep*1000)/1000,
		
		dataColor: {fill: '#0000FF', stroke: '#0000FF', strokeWidth: 1}
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
    max += Math.abs(max * .1);
    min -= Math.abs(min * .1);
		
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
		  
		xaxis: {
			domain: [min, max],
			range: [min, max],
			tickmode: "linear",
			ticks: "outside",
			showticklabels: true
		},
		
		yaxis: {
			domain: [min, max],
			range: [min, max],
			tickmode: "linear",
			ticks: "outside",
			showticklabels: true
		}
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