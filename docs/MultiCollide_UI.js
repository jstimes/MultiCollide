$(function() {
		  
	$("#shapesAccordion").accordion({
	  collapsible: true
	});
	
	var customSuperquadricDialog = $("#customSuperquadricModal").dialog({
		autoOpen: false,
		height: 400,
		width: 400,
		modal: true,	
		buttons: {
			Cancel: function() {
				customSuperquadricDialog.dialog( "close" );
			}
		},
		close: function() {
			
		}
	});
	
	var customPolygonDialog = $("#customPolygonModal").dialog({
		autoOpen: false,
		height: 400,
		width: 400,
		modal: true,	
		buttons: {
			Cancel: function() {
				customPolygonDialog.dialog( "close" );
			}
		},
		close: function() {
			
		}
	});
	
	var meshDialog = $("#meshModal").dialog({
		autoOpen: false,
		height: 400,
		width: 400,
		modal: true,	
		buttons: {
			Cancel: function() {
			  meshDialog.dialog( "close" );
			}
		},
		close: function() {
		
		}
	});
	
	$("#3DaddShapeMenu").menu().on("mouseleave", function(){
		$("#3Dsubmenu").hide();
	}).on("mouseenter", function() {
		$("#3Dsubmenu").show();
	});
	
	$("#2DaddShapeMenu").menu().on("mouseleave", function(){
		$("#2Dsubmenu").hide();
	}).on("mouseenter", function() {
		$("#2Dsubmenu").show();
	});
	
	var isSettingUpScene = true;
 
	var addShape = function(offset) {
		console.log("AddShape");
		if(!offset){
			offset = 1;
		}
		 var shapeIndex = _getNumShapes()-offset;
		 var shapeName = Pointer_stringify(_getShapeName(shapeIndex));
		 
		 var shape = getShapeData(shapeIndex);
		 
		 var html = "<h4 id='shapeHeader" + shapeName + "'>" + shapeName + "</h4>" +
			"<div class='tabs' id='shapeAccordion" + shapeName + "'>" +
			"<ul><li><a href='#physical'>Physical Constants</a></li>" +
			"<li><a href='#orientation'>Orientation</a></li>" +
			"<li><a href='#movement'>Velocities</a></li></ul>" +
			
			"<div id='orientation'>" +
			"<b>Position:</b><br>" +
			"x: <input style='width:50px' type='text' class='positionx shapeData " + shapeName + "' value='" + shape["positionx"] + "'><br>" +
			"y: <input style='width:50px' type='text' class='positiony shapeData " + shapeName + "'><br>";
			
			if(!using2D){
				html += "z: <input style='width:50px' type='text' class='positionz shapeData " + shapeName + "'>";
			}
			
			html += "<br><br>" +
			
			"<b>Rotation</b><br>";
			
			if(!using2D) {
				html += "Axis x: <input style='width:50px' type='text' class='rotationAxisX shapeData " + shapeName + "' value='0'><br>" +
				"Axis y: <input style='width:50px' type='text' class='rotationAxisY shapeData " + shapeName + "' value='0'><br>" +
				"Axis z: <input style='width:50px' type='text' class='rotationAxisZ shapeData " + shapeName + "' value='0'><br>";
			}
			
			html += "Angle: <input style='width:50px' type='text' class='rotationAngle shapeData " + shapeName + "' value='0'><br><br>" +
			
			"</div>" +
	
			"<div id='physical'>" +
			"<b>Scale: </b><input style='width:50px' type='text' class='scale shapeData " + shapeName + "'><br><br>" +
	
			"<b>Mass: </b><input style='width:50px' type='text' class='mass shapeData " + shapeName + "'><br><br>" +
	
			"<b>Friction Coefficient: </b><input style='width:50px' type='text'  class='friction shapeData " + shapeName + "'><br><br>" +
			
			"<b>Restitution Coefficient: </b><input style='width:50px' type='text'  class='restitution shapeData " + shapeName + "'><br><br>" +
			
			"<input type='button' value='Compute Angular Inertia' class='computeInertia " + shapeName + "'><br>" +
			"<span class='angularInertia shapeData " + shapeName + "'></span><br><br>" +
			"</div>" + 

			
			
			"<div id='movement'>" +
			"<b>Velocity:</b><br>" +
			"x: <input style='width:50px' type='text' class='velocityx shapeData " + shapeName + "'><br>" +
			"y: <input style='width:50px' type='text' class='velocityy shapeData " + shapeName + "'><br>";
			
			if(!using2D){
				html += "z: <input style='width:50px' type='text' class='velocityz shapeData " + shapeName + "'>";
			}
			
			html += "<br><br>" + 
			"<b>Angular Velocity:</b><br>";
			
			if(!using2D){
				html += "Rotation axis: <br>" +
				"x: <input style='width:50px' type='text' class='angularvelocityx shapeData " + shapeName + "'><br>" +
				"y: <input style='width:50px' type='text' class='angularvelocityy shapeData " + shapeName + "'><br>" +
				"z: <input style='width:50px' type='text' class='angularvelocityz shapeData " + shapeName + "'><br>";
			}
			
			html += "speed: <input style='width:50px' type='text' class='angularvelocityspeed shapeData " + shapeName + "'><br><br>" +
			"<input type='button' class='removeShape' value='Remove Shape'>" +
			"<input type='button' value='Add Duplicate'>" +
			"</div>" + 
			
			"</div></div>";
			
		$('#shapesAccordion').append(html);
		
		
		$("#shapesAccordion :input:not(:button)").spinner({
			step: 0.001,
			numberFormat: "n"
		});
		
		$(".removeShape").click(function() {
			var namePtr = allocate(intArrayFromString(shapeName), 'i8', ALLOC_NORMAL);
			var index = _getShapeIndexForName(namePtr);
			_removeShape(index);
			$("#shapeAccordion" + shapeName).remove();
			$("#shapeHeader" + shapeName).remove();
			//$('#shapesAccordion').accordion("refresh");
			_free(namePtr);
		});
		
		$(".computeInertia").click(function() {
			var namePtr = allocate(intArrayFromString(shapeName), 'i8', ALLOC_NORMAL);
			var index = _getShapeIndexForName(namePtr);
			_free(namePtr);
			
			var xx = _getShapeAngularInertiaXX(index);
			var xy = _getShapeAngularInertiaXY(index);
			var xz = _getShapeAngularInertiaXZ(index);
			var yx = _getShapeAngularInertiaYX(index);
			var yy = _getShapeAngularInertiaYY(index);
			var yz = _getShapeAngularInertiaYZ(index);
			var zx = _getShapeAngularInertiaZX(index);
			var zy = _getShapeAngularInertiaZY(index);
			var zz = _getShapeAngularInertiaZZ(index);
			
			var table = "Angular Inertia: <br><table class='matrix'>" + 
				"<tr>" + 
					"<td>" + xx + "</td>" + 
					"<td>" + xy + "</td>" + 
					"<td>" + xz + "</td>" + 
				"</tr>" + 
				"<tr>" + 
					"<td>" + yx + "</td>" + 
					"<td>" + yy + "</td>" + 
					"<td>" + yz + "</td>" + 
				"</tr>" + 
				"<tr>" + 
					"<td>" + zx + "</td>" + 
					"<td>" + zy + "</td>" + 
					"<td>" + zz + "</td>" + 
				"</tr>" + 
				"</table>";
				
			$(".angularInertia").append(table);
			$(".computeInertia").hide();
			
			console.log(shapeName);
		});
		
		$(/*".shapeData " + */"." + shapeName).on('change textInput input', function () {
			var dataElement = $(this).attr("class").split(" ")[0];
			var value = Number($(this).spinner("value"));
			
			console.log("text changed for " + shapeName);
			
			var namePtr = allocate(intArrayFromString(shapeName), 'i8', ALLOC_NORMAL);
			var index = _getShapeIndexForName(namePtr);
			_free(namePtr);
			
			if(dataElement === "positionx") {
				_setShapePositionX(index, value);
			} else if(dataElement === "positiony"){
				_setShapePositionY(index, value);
			} else if(dataElement === "positionz"){
				_setShapePositionZ(index, value);
			} else if(dataElement === "velocityx"){
				_setShapeVelocityX(index, value);
			} else if(dataElement === "velocityy"){
				_setShapeVelocityY(index, value);
			} else if(dataElement === "velocityz"){
				_setShapeVelocityZ(index, value);
			} else if(dataElement === "angularvelocityx"){
				_setShapeAngularVelocityX(index, value);
			} else if(dataElement === "angularvelocityy"){
				_setShapeAngularVelocityX(index, value);
			} else if(dataElement === "angularvelocityz"){
				_setShapeAngularVelocityX(index, value);
			} else if(dataElement === "angularvelocityspeed"){
				_setShapeAngularVelocitySpeed(index, value);
			} else if(dataElement === "scale"){
				_setShapeScale(index, value);
			} else if(dataElement === "mass"){
				_setShapeMass(index, value);
			} else if(dataElement === "friction"){
				_setShapeFriction(index, value);
			} else if(dataElement === "restitution"){
				_setShapeRestitution(index, value);
			} else if(dataElement === "rotationAxisX"){
				_setShapeRotationAxisX(index, value);
			}
			else if(dataElement === "rotationAxisY"){
				_setShapeRotationAxisY(index, value);
			}
			else if(dataElement === "rotationAxisZ"){
				_setShapeRotationAxisZ(index, value);
			}
			else if(dataElement === "rotationAngle"){
				_setShapeRotationAngle(index, value);
			}
		});
		
		$('.ui-spinner-button').click(function() {
		   $(this).siblings('input').change();
		});
		
		$(".tabs").tabs().addClass( "ui-tabs-vertical ui-helper-clearfix" );
		$(".tabs li").removeClass( "ui-corner-top" ).addClass( "ui-corner-left" );
		
		$('#shapesAccordion').accordion("refresh");
			
		//dialog.dialog("close");
	 }
	
	$( ".cursorState" ).checkboxradio();
	
	$("#collisionMode").change(function() {
		_setCollisionMode(this.checked);
		localStorage.setItem('freefly', this.checked);
	});
	
	$("input[name=rotationDisplay]").change(function() {
		var value = $("input[name=rotationDisplay]:checked").val();
		localStorage.setItem('rotationDisplay', value);
		configureRotationDisplay(value);
	});
	
	$("#cameraPan").click(function() {
		_setCameraStatePan();
	});
	
	$("#cameraRotate").click(function() {
		_setCameraStateRotate();
	});
	
	$("#cameraZoom").click(function() {
		_setCameraStateZoom();
	});
	
	$('#addSphereBtn').click(function() {
		_AddSphereOnClick();
		addShape();
		console.log("add sphere btn");
	});
	
	$('#addCubeBtn').click(function() {
		_AddCubeOnClick();
		addShape();
	});
	
	$('#addEllipsoidBtn').click(function() {
		_AddEllipsoidOnClick();
		addShape();
	});
	
	$('#addSuperquadricBtn').click(function() {
		_AddSuperquadricOnClick();
		addShape();
	});
	
	$('#addCustomSuperquadricBtn').click(function() {
		customSuperquadricDialog.dialog("open");
	});
	
	$("#addMeshBtn").click(function() {
		meshDialog.dialog("open");
	});
	
	$("#addIcoBtn").click(function() {
		_AddIcosahedronOnClick();
		addShape();
		addShape(2); //Both the tetra and Icosahedron are added
	});
	
	$("#addCircleBtn").click(function() {
		_AddCircleOnClick();
		addShape();
	});
	
	$("#addTriangleBtn").click(function() {
		_AddTriangleOnClick();
		addShape();
	});
	
	$("#addSquareBtn").click(function() {
		_AddSquareOnClick();
		addShape();
	});
	
	$("#addPolygonBtn").click(function() {
		customPolygonDialog.dialog("open");
	});
	
	$("#addPolygon").click(function() {
		customPolygonDialog.dialog("close");
		var numSides = parseInt($("#numSides").val());
		if(numSides < 3 || numSides > 99){
			alert("Invalid number of sides");
			return;
		}
		_AddPolygonOnClick(numSides);
		addShape();
	});
	
	$("#meshFileUpload").change(function() {
		if(this.files != null && this.files.length > 0){
			var fileName = this.files[0].name;
			var reader = new FileReader ();
			reader.onloadend = function (ev) { 
				
				var fileNamePtr = allocate(intArrayFromString(fileName), 'i8', ALLOC_NORMAL);
				var fileContentPtr = allocate(intArrayFromString(this.result), 'i8', ALLOC_NORMAL);
				
				_uploadMesh(fileNamePtr, fileContentPtr); 
				addShape();
				
				_free(fileContentPtr);
				_free(fileNamePtr);
				
				meshDialog.dialog("close");
			
			};
			reader.readAsText (this.files[0]);
		}
	});
	
	$('#run').click(function() {
		_RunOnClick();
		
		if(isSettingUpScene){
			//Clicked run:
			isSettingUpScene = !isSettingUpScene;
			
			$("#run").attr("value", "Pause");
			$("#reset").attr("value", "Stop");
		}
		else {
			//Clicked pause/play:
			_PauseOnClick();
			$("#run").attr("value", ($("#run").attr("value") == "Play" ? "Pause" : "Play"));
		}
	 });
	 
	 $("#addCustom").click(function() {
		 var a1 = parseFloat($("#a1").val());
		var a2 = parseFloat($("#a2").val());
		var a3 = parseFloat($("#a3").val());
		var e1 = parseFloat($("#e1").val());
		var e2 = parseFloat($("#e2").val());
		//console.log("e2: " + e2);
		_createCustomSuperquadric(a1, a2, a3, e1, e2);	
		addShape();
		customSuperquadricDialog.dialog("close");
	 });
	 
	 $("#reset").click(function() {
		
		if(isSettingUpScene){
			//Clicked reset:
			_ResetOnClick();
			var myNode = document.getElementById("shapesAccordion");
			while (myNode.firstChild) {
				myNode.removeChild(myNode.firstChild);
			}
		}
		else {
			//Clicked stop:
			if(isImpulseMode){
				isImpulseMode = false;
				_continueImpulseOnClick();
				toggleImpulseControls();
			}
			
			isSettingUpScene = !isSettingUpScene;
			_StopOnClick();
			$("#run").attr("value", "Run");
			$("#reset").attr("value", "Reset Scene");
		}
	});
	
	$("#resetCamera").click(function() {
		_resetCameraState();
	});
	
	document.getElementById('translate').checked = true;
	$(".cursorState").checkboxradio( "refresh" );
	
	$("#sceneShapesShowBtn").click(function() {
		
		showSceneShapes();
		
		
		
		
		// $("#rightSidePanel").show();
		// window.dispatchEvent(new Event('resize'));
		// var toGrow = $("#rightSidePanel").width();
		// $("#rightSidePanel").width(0);
		
		// $.when($("#rightSidePanel").animate( {
			// width: toGrow
		// }, { duration: 2000, queue: false })).then(/*function() { window.dispatchEvent(new Event('resize')); }*/);
		
		// var newWidth = $("#mainContent").width() - toGrow;
		// $("#mainContent").animate( {
			// width: newWidth
		// }, { duration: 2000, queue: false }, 
		
		
		
		
		
		/*
		$("#rightSidePanel").show("slide", { direction: "right" }, 1000);*/
		
		
		//$.when($("#rightSidePanel").show("slide", { direction: "right" }, 1000)).then(function() {
			//sceneShapesOpen = true;
			//window.dispatchEvent(new Event('resize'));
		//});
	});
	
	$("#sceneShapesHideBtn").click(function() {
		$("#rightSidePanel").hide("slide", { direction: "right" }, 1000);
		$.when($("#sceneShapesShowBtnDiv").show("slide", { direction: "right" }, 1000)).then(function() {
			sceneShapesOpen = false;
			window.dispatchEvent(new Event('resize'));
		});
	});
	
	
	$(":radio[name=dimension]").change(function() {
		var newMode = $(":radio[name=dimension]:checked").val();
		
		localStorage.setItem('simMode', newMode);
		
		initDimension(newMode);
	});
	
	$("#showImpulse").change(function() {
		showImpulseGraph = this.checked;
		localStorage.setItem('showImpulse', this.checked);
		checkIfShouldShowGraphs();
	});
	
	$("#showVelocity").change(function() {
		showVelocityGraph = this.checked;
		localStorage.setItem('showVelocity', this.checked);
		checkIfShouldShowGraphs();
	});
		  
});

var sceneShapesOpen = true;
 
 window.onresize = function() {
	  if(document["fullScreen"])
		  return;
	// var w = window.innerWidth;
	 var h = window.innerHeight;
	
	 var topOffset = document.getElementById('top').clientHeight + 8;
	// console.log("Top offset  " + topOffset);
	 h = h - topOffset;
	
	// var leftOffset = document.getElementById('leftSidePanel').clientWidth + 4;
	// var rightOffset = document.getElementById('rightSidePanel').clientWidth + 4;
	// w = w - rightOffset - leftOffset;
	
	//.position().left and .offsetWidth
	
	var rightPx = -1;
	if(sceneShapesOpen){
		rightPx = $("#rightSidePanel").position().left - 5;
	}
	else {
		rightPx = window.innerWidth - 5;//$("#sceneShapesShowBtnDiv").position().left - 5;
	}
	
	//var leftPx = $("#leftSidePanel").position.left + $("#leftSidePanel").position().offsetWidth;
	var leftPx = 150 + 5;

	var w = rightPx - leftPx;
	
	if(typeof GLFW !== 'undefined'){
		//document.getElementById('impulseControls').width = w;
		//document.getElementById('impulseControls').width = w;
		document.getElementById('mainContent').width = w;
		
		document.getElementById('mainContent').height = h;
		document.getElementById('canvas').width = w;
		if(h > w){
			h = w;
		}
		console.log("canvas was " + $("#canvas").height());
		document.getElementById('canvas').height = h;
		console.log("canvas is " + $("#canvas").height());
		 GLFW.setWindowSize(1, w, 
			h);
	 }
  }

 
 document.getElementById('translate').onclick = function() {
		_TranslateOnClick();
 }
 
  document.getElementById('rotate').onclick = function() {
		_RotateOnClick();
 }
 
  document.getElementById('scale').onclick = function() {
		_ScaleOnClick();
 }
 
  document.getElementById('velocity').onclick = function() {
		_VelocityOnClick();
 }
 
 document.getElementById('angularVelocity').onclick = function() {
		_AngularVelocityOnClick();
 }
 
 function getShapeData(i){
	 var data = [];

	data["positionx"] = _getShapePositionX(i);
	data["positiony"] = _getShapePositionY(i);
	data["positionz"] = _getShapePositionZ(i);
	data["velocityx"] = _getShapeVelocityX(i);
	data["velocityy"] = _getShapeVelocityY(i);
	data["velocityz"] = _getShapeVelocityZ(i);
	data["mass"] = _getShapeMass(i);
	data["scale"] = _getShapeScale(i);
	data["friction"] = _getShapeFriction(i);
	data["angularvelocityx"] = _getShapeAngularVelocityX(i);
	data["angularvelocityy"] = _getShapeAngularVelocityY(i);
	data["angularvelocityz"] = _getShapeAngularVelocityZ(i);
	data["angularvelocityspeed"] = _getShapeAngularVelocitySpeed(i);
	data["restitution"] = _getShapeRestitution(i);
	
	data["rotationAxisX"] = _getShapeRotationAxisX(i);
	data["rotationAxisY"] = _getShapeRotationAxisY(i);
	data["rotationAxisZ"] = _getShapeRotationAxisZ(i);
	data["rotationAngle"] = _getShapeRotationAngle(i);
	
	return data;
 }
 
 function updateShapeData() {
	$("#shapesAccordion div.tabs").each(function(index, divElement) {
		//console.log("Shape index: " + index);
		var shapeData = getShapeData(index);
		$(divElement).find("input:not(:button)").each(function() {
			var dataElement = $(this).attr("class").split(" ")[0];
			//console.log(dataElement);
			$(this).spinner("value", shapeData[dataElement]);
		});
	});
 }
 
 var showImpulseGraph = false;
 var showVelocityGraph = false;
 var isImpulseMode = false;
 
 var impulsePts = [];
 var velocityPts = [];
 
 function toggleImpulseControls() {
	if(!isImpulseMode) {
        //$('#impulseControls').hide();//"slide", { direction: "right" }, 1000);
		//console.log("width: " + $('#impulseControls').width());
		//window.dispatchEvent(new Event('resize'));
		
		$.when($("#graphsDiv").hide("slide", { direction: "right" }, 1000)).then(function() {
			$("#sceneShapesDiv").show("slide", { direction: "right" }, 1000);
			
		});
	}
    else {
		// console.log("window ht: " + window.innerHeight);
		// console.log("canvas ht: " + $('#canvas').height());
		// console.log("main c ht: " + $('#mainContent').height());
		// console.log("impulse controls w: " + $("#impulseControls").width());
		// if($('#canvas').height() < $('#mainContent').height()){
			// console.log("smaller");
		// }
		// else {
			// console.log("bigger");
		// }
		
		// var controlsHeight = 40000 / $("#canvas").width();
		// if(controlsHeight < 50){
			// controlsHeight = 50;
		// }
		
		// if(window.innerHeight - $("#mainContent").height() - $("#top").height() < controlsHeight){
			// var curHeight = $("#canvas").height();
			// //$("#canvas").height(curHeight - controlsHeight);
			// document.getElementById("canvas").height = curHeight - controlsHeight;
			// GLFW.setWindowSize(1, $("#canvas").width(), $("#canvas").height());
			
			// console.log("resized canvas");
		// }
		
		
        //$('#impulseControls').show();//"slide", { direction: "right" }, 1000);
		//$('#impulseControls').width($('#canvas').width());
		//console.log("IC width: " + $('#impulseControls').width());
		
		velocityPts = [];
		while(_isMoreVelocityPts()){
			velocityPts.push(_getNextVelocityPtX());
			velocityPts.push(_getNextVelocityPtY());
			velocityPts.push(_getNextVelocityPtZ());
		}
		
		impulsePts = [];
		while(_isMoreImpulsePts()){
			impulsePts.push(_getNextImpulsePtX());
			impulsePts.push(_getNextImpulsePtY());
			impulsePts.push(_getNextImpulsePtZ());
		}
		
		var v1EndX = round3(_getImpactResultsV1endX());
		var v1EndY = round3(_getImpactResultsV1endY());
		var v1EndZ = round3(_getImpactResultsV1endZ());
	
		var v2EndX = round3(_getImpactResultsV2endX());
		var v2EndY = round3(_getImpactResultsV2endY());
		var v2EndZ = round3(_getImpactResultsV2endZ());
		
		var w1EndX = round3(_getImpactResultsW1endX());
		var w1EndY = round3(_getImpactResultsW1endY());
		var w1EndZ = round3(_getImpactResultsW1endZ());
		
		var w2EndX = round3(_getImpactResultsW2endX());
		var w2EndY = round3(_getImpactResultsW2endY());
		var w2EndZ = round3(_getImpactResultsW2endZ());
		
		var endOfSliding = _getImpactResultsEndOfSliding();
		var endOfCompres = _getImpactResultsEndOfCompression();
		
		var IendX = round3(_getImpactResultsIendX());
		var IendY = round3(_getImpactResultsIendY());
		var IendZ = round3(_getImpactResultsIendZ());
		
		
		//TODO if 2D don't output z's
		var html = "<h4 id='impactVisualizations'>A Collision Occurred</h4>" + 
			"<div id='graphsContainer' height='400px'>" + 
				"<p class='graphLabel'>Impulse Accumulation</p>" + 
				"<div id='graph1'></div><br><br>" + 
				
				"<p class='graphLabel'>Sliding Velocity Curve (Hodograph)</p>" +
				"<div id='graph2'></div>" +
				
				"<p>Total Impulse: <br>" + 
				"(" + IendX + ", " + IendY + ", " + IendZ + ")<br>" + 
				
				"<p>End of Sliding at: " + endOfSliding + "</p><br>" +
				"<p>End of Compression at: " + endOfCompres + "</p>" +
				
				"<p>Post impact velocities: <br>" + 
				"(" + v1EndX + ", " + v1EndY + ", " + v1EndZ + ")<br>" + 
				"(" + v2EndX + ", " + v2EndY + ", " + v2EndZ + ")<br>" + 
				
				"<p>Post impact angular velocities: <br>" + 
				"(" + w1EndX + ", " + w1EndY + ", " + w1EndZ + ")<br>" + 
				"(" + w2EndX + ", " + w2EndY + ", " + w2EndZ + ")<br><br>" + 
				
				"<input type='button' class='ui-button ui-widget ui-corner-all' id='impulseContinueBtn' value='Continue'>" +
				"<input type='button' class='ui-button ui-widget ui-corner-all' id='impulseRestartBtn' value='Restart'>" +
			"</div>";
		
		var accord = $('#shapesAccordion');
		accord.append(html);
		accord.accordion("refresh");
		// $("#rightSidePanel").attr('height', 400);
		// accord.attr('height', 400);
		
		$("#impulseContinueBtn").click(function() {
			stopVisualizations();
			_continueImpulseOnClick();
			$("#graphsContainer").remove();
			$("#impactVisualizations").remove();
		});
		
		$("#impulseRestartBtn").click(function() {
			stopVisualizations();
			$("#graph1").empty();
			$("#graph2").empty();
			drawVisualizations(impulsePts, velocityPts);
		});
		
		var showGraph = function() {
			var indexToOpen = accord.children('div').length - 1;
			accord.accordion('option', 'active', indexToOpen);
			drawVisualizations(impulsePts, velocityPts);
		};
		
		if(!sceneShapesOpen){
			showSceneShapes(showGraph);
			//If not already open, need to wait til it's on screen
			// before drawing graphs so the height isn't still 0
			
			//TODO ^ not working
		}
		else {
			showGraph();
		}
		
		
		/*
		//12/16 vis.js 
		drawVisualizations(impulsePts, velocityPts);
		if(!sceneShapesOpen){
			$("#sceneShapesDiv").hide();
			$("#graphsDiv").show();//"slide", { direction: "right" }, 1000);
			$("#sceneShapesShowBtn").click();
		}
		else {
			
		
			$.when($("#sceneShapesDiv").hide("slide", { direction: "right" }, 1000)).then(function() {
				$("#graphsDiv").show("slide", { direction: "right" }, 1000);
				
			});
			
			//TODO outuput v1end, v2end, w1end, w2end, Iend, EOC, EOS
		}
		*/
		
	}
 }
 
 function round3(decimal) {
	 return Math.round(decimal * 1000) / 1000;
 }
 
 function showSceneShapes(callback) {
	$.when($("#sceneShapesShowBtnDiv").hide("slide", { direction: "right" }, 700)).then(function() {
		sceneShapesOpen = true;
		
		$("#rightSidePanel").show();
		window.dispatchEvent(new Event('resize'));
		var toGrow = $("#rightSidePanel").width();
		$("#rightSidePanel").width(0);
		
		$.when($("#rightSidePanel").animate( {
			width: toGrow
		}, { duration: 2000, queue: false })).then(callback);
	});
}
 
 function checkIfShouldShowGraphs(){
	 if(!showImpulseGraph && !showVelocityGraph){
		_generateGraphs(false);
	}
	else {
		_generateGraphs(true);
	}
 }
 
 function configureRotationDisplay(rotDisplay) {
	_setPlotRotaionAxis(false);
	_setPlotPricipalFrame(false);
	if(rotDisplay === 'principalFrame'){
		_setPlotPricipalFrame(true);
	}
	else if(rotDisplay === 'rotationAxis'){
		_setPlotRotaionAxis(true);
	}	
 }
 
 var using2D = false;
 
 //Toggles between 2D/3D interface mode
 function initDimension(simMode){
	 if(simMode === '3D'){
		_use2DShapes(false);
		$("#2DaddShapeMenu").hide();
		$("#3DaddShapeMenu").show();
		
		$("#cameraDomToHide").show();
		
		$("#angularVelocity").show();
		
		using2D = false;
	 }
	 else {
		_use2DShapes(true);
		$("#2DaddShapeMenu").show();
		$("#3DaddShapeMenu").hide();
		
		$("#cameraDomToHide").hide();
		
		$("#angularVelocity").hide();
		
		using2D = true;
	 }
 }
 
 function initSettings() {
	window.dispatchEvent(new Event('resize'));
	 
	//Simulation Mode:
	var simMode = localStorage.getItem('simMode');
	$(":radio[name=dimension][value='" + simMode + "']").prop("checked", true);
	initDimension(simMode);
	
	//Collision Mode:
	var collisionMode = (localStorage.getItem('freefly') === 'true');
	$("#collisionMode").prop("checked", collisionMode);
	_setCollisionMode(collisionMode);
	
	//Rotation display:
	var rotDisplay = localStorage.getItem('rotationDisplay');
	$(":radio[name=rotationDisplay][value='" + rotDisplay + "']").prop('checked', true);
	configureRotationDisplay(rotDisplay);
	
	//Graph display:
	showImpulseGraph = (localStorage.getItem('showImpulse') === 'true');
	$("#showImpulse").prop('checked', showImpulseGraph);
	
	var showVelocityGraph = (localStorage.getItem('showVelocity') === 'true');
	$("#showVelocity").prop('checked', showVelocityGraph);
	checkIfShouldShowGraphs();
 }
 
 function initSettingsStorage() {
	 localStorage.setItem('simMode', '3D');
	 localStorage.setItem('freefly', true);
	 localStorage.setItem('rotationDisplay', 'noRotationDisplay');
	 localStorage.setItem('showImpulse', true);
	 localStorage.setItem('showVelocity', true);
 }