function drawGraph2() {
	var data = null;
    var graph = null;
	drawVisualization();

    function custom(x, y, t) {
      return Math.sin(x/50 + t/10) * Math.cos(y/50 + t/10) * 50 + 50;
    }

    // Called when the Visualization API is loaded.
    function drawVisualization() {
      // Create and populate a data table.
      data = new vis.DataSet();
      // create some nice looking data with sin/cos
      var steps = 25;
      var axisMax = 314;
      var tMax = 31;
      var axisStep = axisMax / steps;
      for (var t = 0; t < tMax; t++) {
        for (var x = 0; x < t; x+=axisStep) {
			
			var y = x - 15;
			var z = 0;
			
            data.add([
              {x:x,y:y,z:z,filter:t}
            ]);
        }
      }

      // specify options
      var options = {
        width:  '220px',
        height: '220px',
        style: 'line',
        showPerspective: true,
        showGrid: true,
        showShadow: false,
        // showAnimationControls: false,
        keepAspectRatio: true,
        verticalRatio: 0.5,
        animationInterval: 100, // milliseconds
        animationPreload: true,
		animationAutoStart: true,
        filterValue: 'time'
      };

      // create our graph
      var container = document.getElementById('graph2');
      graph = new vis.Graph3d(container, data, options);
    }
  }