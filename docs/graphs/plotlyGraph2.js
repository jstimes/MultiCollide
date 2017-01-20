function grah() {
	var pointCount = 31;
	var i, r;

	var x = [];
	var y = [];
	var z = [];
	var c = [];

	for(i = 0; i < pointCount; i++) 
	{
	   r = 10 * Math.cos(i / 10);
	   x.push(r * Math.cos(i));
	   y.push(r * Math.sin(i));
	   z.push(i);
	   c.push(i)
	}

	console.log(document.getElementById('thegraph').innerHTML);

	Plotly.plot('thegraph', [{
	  type: 'scatter3d',
	  mode: 'lines+markers',
	  x: x,
	  y: y,
	  z: z,
	  line: {
		width: 6,
		color: c,
		colorscale: "Viridis"},
	  marker: {
		size: 3.5,
		color: c,
		colorscale: "Greens",
		cmin: -20,
		cmax: 50
	  }},                  
	], {
	  height: 220,
	  width: 220
	 });

	Plotly.d3.csv('https://raw.githubusercontent.com/plotly/datasets/master/3d-line1.csv', function(err, rows){
		  function unpack(rows, key) {
			  return rows.map(function(row) 
			  { return row[key]; }); 
			}
			
	var x = unpack(rows , 'x');
	var y = unpack(rows , 'y');
	var z = unpack(rows , 'z'); 
	var c = unpack(rows , 'color');

	var data = [{
	  type: 'scatter3d',
	  mode: 'lines',
	  x: x,
	  y: y,
	  z: z,
	  opacity: 1,
	  line: {
		width: 6,
		color: c,
		reversescale: false
	  }
	 }];
	 
	 var layout = {
	  autosize: false,
	  width: 350,
	  height: 350,
	  margin: {
		l: 50,
		r: 50,
		b: 100,
		t: 100,
		pad: 4
	  },
	  paper_bgcolor: '#7f7f7f',
	  plot_bgcolor: '#c7c7c7'
	};
	 
	Plotly.plot('graphBig', data, layout);

});


}