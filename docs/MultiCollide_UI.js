function multicollide_init() {
	
	if(!localStorage.getItem('simMode')){
		initSettingsStorage();
	}
	initSettings();
		  
	$("#shapesAccordion").accordion({
	  collapsible: true,
	  heightStyle: "fill"
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
	
	var numCustomVertices = 3;
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
			numCustomVertices = 3;
		}
	});
	
	var uploadSceneDialog = $("#uploadSceneModal").dialog({
		autoOpen: false,
		height: 400,
		width: 400,
		modal: true,	
		buttons: {
			Cancel: function() {
			  uploadSceneDialog.dialog( "close" );
			}
		}
	});
	
	var impactSettingsDialog = $("#impactSettingsModal").dialog({
		autoOpen: false,
		height: 400,
		width: 400,
		modal: true,	
		buttons: {
			Cancel: function() {
			  impactSettingsDialog.dialog( "close" );
			},
			"Save": function() {
				var newFriction = $("#friction").val();
				var newRestitution = $("#restitution").val();
				_setShapeFriction(newFriction);
				_setShapeRestitution(newRestitution);
				impactSettingsDialog.dialog( "close" );
			}
		},
		close: function() {
		
		}
	});
	
	$("#addCustomPolygonBtn").click(function() {
		
		var pts = [];
		//var lastPt = {};
		var ptsText = $("#customVerticesTextArea").val();
		var ptsTextArr = ptsText.split(/\r?\n/);
		for(var i=0; i<ptsTextArr.length; i++){
			var xyArr = ptsTextArr[i].split(',');
			pts.push({x: parseFloat(xyArr[0]), y: parseFloat(xyArr[1])});
		}
		/*
		$(".customVertices").each(function() {
			if($(this).hasClass('xcoord')){
				lastPt = {x: $(this).val() };
			}
			else {
				lastPt.y = $(this).val();
				pts.push(lastPt);
			}
		});*/
		
		//console.log(pts);
		_createNewCustomPolygon();
		for(var i=0; i<pts.length; i++){
			_addCustomPolygonVertex(pts[i].x, pts[i].y);
		}
		_doneCreatingCustomPolygon();
		addShape();
	});
	
	$("#addAnotherVertexBtn").click(function(){
		var html = $("<li>").append("(x, y): ").append( $("<input>", {
			class: 'customVertices xcoord',
			type: 'text'
		})).append(", ").append($("<input>", {
			class: 'customVertices ycoord',
			type: 'text'
		}));
		$("#customVerticesList").append(html);
	});
	
	$("#removeVertexBtn").click(function() {
		$("#customVerticesList li").last().remove();
	});
	
	// $("#impactSettingsBtn").click(function() {
		// $("#friction").val(_getShapeFriction());
		// $("#restitution").val(_getShapeRestitution());
		
		// impactSettingsDialog.dialog("open");
		// slideTop.close();
	// });
	
	$("#uploadSceneBtn").click(function() {
		uploadSceneDialog.dialog("open");
	});
	
	$("#submitSceneBtn").click(function(){
		var selectedFile = document.getElementById('sceneFileUpload').files[0];
		
		var reader = new FileReader ();
		reader.onloadend = function (ev) { 
			var fileContents = this.result;
			parseSceneCSV(fileContents);
			uploadSceneDialog.dialog("close");
		
		};
		reader.readAsText(selectedFile);
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
		 
		 var sectionId = "shapeAccordion" + shapeName;
		 
		 var html = "<h4 id='shapeHeader" + shapeName + "'>" + shapeName;
		    //"<div class='close-button'></div>" +
			//<input type='button' value='Remove' class='removeShape close-button'/>
			
			if(!icoMode){
				html += "<span class='removeButton'>&times;</span>";
				//html += "<span class='duplicateButton'>&#43; Duplicate</span>";
			}
			
			html += "</h4>" +
			"<div class='tabs' id='" + sectionId + "'>" +
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
			
			"<b>Rotation:</b><br>";
			
			if(!using2D) {
				html += "Axis x: <input style='width:50px' type='text' class='rotationAxisX shapeData " + shapeName + "' value='0'>" +
				//"<span class='normalizedRotAxisX'>0</span>"+
				"<br>" +
				"Axis y: <input style='width:50px' type='text' class='rotationAxisY shapeData " + shapeName + "' value='0'><br>" +
				"Axis z: <input style='width:50px' type='text' class='rotationAxisZ shapeData " + shapeName + "' value='0'><br>";
			}
			
			html += "Angle: <input style='width:50px' type='text' class='rotationAngle shapeData " + shapeName + "' value='0'><br><br>" +
			
			"</div>" +
	
			"<div id='physical'>" +
			"<b>Scale: </b><input style='width:50px' type='text' class='scale shapeData " + shapeName + "'><br><br>" +
	
			"<b>Mass: </b><input style='width:50px' type='text' class='mass shapeData " + shapeName + "'><br><br>" +
			
			//"<input type='button' value='Compute Angular Inertia' class='computeInertia " + shapeName + "'><br>" +
			"<span class='angularInertia shapeData " + shapeName + "'></span><br><br>" +
			"</div>" + 
			
			"<div id='movement'>" +
			"<b>Velocity:</b><br>" +
			"x: <input style='width:50px' type='text' class='velocityx shapeData " + shapeName + "'><br>" +
			"y: <input style='width:50px' type='text' class='velocityy shapeData " + shapeName + "'><br>";
			
			if(!using2D) {
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
			
			//html += "speed (deg/s): <input style='width:50px' type='text' class='angularvelocityspeed shapeData " + shapeName + "'><br><br>" +
			html += "speed (deg/s): <span style='width:50px' type='text' class='angularvelocityspeed " + shapeName + "'>0</span><br><br>" +
			
			"<!--input type='button' class='duplicateShape' value='Add Duplicate'-->" +
			"</div>" + 
			
			"</div></div>";
			
		$('#shapesAccordion').append(html);		
		
		$('#shapesAccordion .removeButton').click(function(event) {
			event.stopImmediatePropagation();
			event.preventDefault();
			
			var namePtr = allocate(intArrayFromString(shapeName), 'i8', ALLOC_NORMAL);
			var index = _getShapeIndexForName(namePtr);
			_removeShape(index);
			$("#shapeAccordion" + shapeName).remove();
			$("#shapeHeader" + shapeName).remove();
			_free(namePtr);
		});
		
		$('#shapesAccordion .removeButton').hover(
			function() {
				$(this).addClass('removeButtonHover');
			},
			function() {
				$(this).removeClass('removeButtonHover');
			}
		);
		
		$('#shapesAccordion .duplicateButton').click(function(event) {
			event.stopImmediatePropagation();
			event.preventDefault();
			
			//TODO
		});
		
		$('#shapesAccordion .duplicateButton').hover(
			function() {
				$(this).addClass('duplicateButtonHover');
			},
			function() {
				$(this).removeClass('duplicateButtonHover');
			}
		);
		
		$("#shapesAccordion :input:not(:button)").spinner({
			step: 0.001,
			numberFormat: "n"
		});
		
		//$("#" + sectionId + " .removeShape").button();
		// $("#" + sectionId + " .removeShape").click(function(event){
			// event.stopPropagation(); //Without this line and the next,
			// event.preventDefault(); 	// callback goes only to accordion, skips btn 
			// alert("clicked!");
		// });

		// $("#" + sectionId + " .removeShape").click(function() {
			// alert("Hey");
			// var namePtr = allocate(intArrayFromString(shapeName), 'i8', ALLOC_NORMAL);
			// var index = _getShapeIndexForName(namePtr);
			// _removeShape(index);
			// $("#shapeAccordion" + shapeName).remove();
			// $("#shapeHeader" + shapeName).remove();
			// _free(namePtr);
			
			// //$('#shapesAccordion').refresh();
		// });
		
		$("#" + sectionId + " .duplicateShape").click(function() {
			var namePtr = allocate(intArrayFromString(shapeName), 'i8', ALLOC_NORMAL);
			var index = _getShapeIndexForName(namePtr);
			
			//TODO add duplicate function to shapes 
			
			_free(namePtr);
		});
		
		//$("#" + sectionId + " .computeInertia").click(function() {
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
			
			var table = "<span>Angular Inertia: </span><br><br>"+
			"<table class='matrix angularInertiaMatrix'>" + 
				"<tr>" + 
					"<td>" + roundNumber(xx) + "</td>" + 
					"<td>" + roundNumber(xy) + "</td>" + 
					"<td>" + roundNumber(xz) + "</td>" + 
				"</tr>" + 
				"<tr>" + 
					"<td>" + roundNumber(yx) + "</td>" + 
					"<td>" + roundNumber(yy) + "</td>" + 
					"<td>" + roundNumber(yz) + "</td>" + 
				"</tr>" + 
				"<tr>" + 
					"<td>" + roundNumber(zx) + "</td>" + 
					"<td>" + roundNumber(zy) + "</td>" + 
					"<td>" + roundNumber(zz) + "</td>" + 
				"</tr>" + 
				"</table>";
				
			$("#" + sectionId + " .angularInertia").append(table);
			$("#" + sectionId + " .computeInertia").hide();
			
			console.log(shapeName);
		//});
		
		$("#" + sectionId + " .shapeData").on('change textInput input', function () {
			//Assumes first class is the data label
			var dataElement = $(this).attr("class").split(" ")[0];
			var value = Number($(this).spinner("value"));
			
			console.log("text changed for " + shapeName);
			
			var namePtr = allocate(intArrayFromString(shapeName), 'i8', ALLOC_NORMAL);
			var index = _getShapeIndexForName(namePtr);
			_free(namePtr);
			
			if(jacobs_frame){
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
					calculateAngularVelocitySpeed(index, sectionId);
				} else if(dataElement === "angularvelocityy"){
					_setShapeAngularVelocityY(index, value);
					calculateAngularVelocitySpeed(index, sectionId);
				} else if(dataElement === "angularvelocityz"){
					_setShapeAngularVelocityZ(index, value);
					calculateAngularVelocitySpeed(index, sectionId);
				//} else if(dataElement === "angularvelocityspeed"){
					//Assume user enters speed in degrees/sec, convert to rads/s 
				//	value *= (Math.PI / 180);
				//	_setShapeAngularVelocitySpeed(index, value);
				} else if(dataElement === "scale"){
					_setShapeScale(index, value);
				} else if(dataElement === "mass"){
					_setShapeMass(index, value);
				} 
				/*else if(dataElement === "friction"){
					_setShapeFriction(index, value);
				} else if(dataElement === "restitution"){
					_setShapeRestitution(index, value);
				}*/ else if(dataElement === "rotationAxisX"){
					_setShapeRotationAxisX(index, value);
					calculateNormalizedRotAxis(index, sectionId);
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
			}
			else {
				if(dataElement === "positionx") {
					_setShapePositionX(index, value);
				} else if(dataElement === "positiony"){
					_setShapePositionZ(index, -1.0 * value);
				} else if(dataElement === "positionz"){
					_setShapePositionY(index, value);
				} else if(dataElement === "velocityx"){
					_setShapeVelocityX(index, value);
				} else if(dataElement === "velocityy"){
					_setShapeVelocityZ(index, -1.0 * value);
				} else if(dataElement === "velocityz"){
					_setShapeVelocityY(index, value);
				} else if(dataElement === "angularvelocityx"){
					_setShapeAngularVelocityX(index, value);
					calculateAngularVelocitySpeed(index, sectionId);
				} else if(dataElement === "angularvelocityy"){
					_setShapeAngularVelocityZ(index, -1.0 * value);
					calculateAngularVelocitySpeed(index, sectionId);
				} else if(dataElement === "angularvelocityz"){
					_setShapeAngularVelocityY(index, value);
					calculateAngularVelocitySpeed(index, sectionId);
				//} else if(dataElement === "angularvelocityspeed"){
					//Assume user enters speed in degrees/sec, convert to rads/s 
				//	value *= (Math.PI / 180);
				//	_setShapeAngularVelocitySpeed(index, value);
				} else if(dataElement === "scale"){
					_setShapeScale(index, value);
				} else if(dataElement === "mass"){
					_setShapeMass(index, value);
				} 
				/*else if(dataElement === "friction"){
					_setShapeFriction(index, value);
				} else if(dataElement === "restitution"){
					_setShapeRestitution(index, value);
				}*/ else if(dataElement === "rotationAxisX"){
					_setShapeRotationAxisX(index, value);
					calculateNormalizedRotAxis(index, sectionId);
				}
				else if(dataElement === "rotationAxisY"){
					_setShapeRotationAxisZ(index, -1.0 * value);
					calculateNormalizedRotAxis(index, sectionId);
				}
				else if(dataElement === "rotationAxisZ"){
					_setShapeRotationAxisY(index, value);
					calculateNormalizedRotAxis(index, sectionId);
				}
				else if(dataElement === "rotationAngle"){
					_setShapeRotationAngle(index, value);
					calculateNormalizedRotAxis(index, sectionId);
				}
			}
		});
		
		$('.ui-spinner-button').click(function() {
		   $(this).siblings('input').change();
		});
		
		$(".tabs").tabs();//.addClass( "ui-tabs-vertical ui-helper-clearfix" );
		$(".tabs li a").css('padding', "2px");//.removeClass( "ui-corner-top" );//.addClass( "ui-corner-left" );
		
		$('#shapesAccordion').accordion("refresh");
		// $("#shapesAccordion").accordion({ active: clicked });
		// $("#shapesAccordion .ui-accordion-header").unbind('click');
		// $("#" + sectionId + " .removeShape").click(function(event) {
			// event.stopPropagation(); //Without this line and the next,
			// event.preventDefault(); 	// callback goes only to accordion, skips btn 
			// alert("clicked!");
		// });
		
		// $("#shapesAccordion .ui-accordion-header").click(function() {
			// var clicked = $(this).attr("tabindex");
			// alert("Clicked index: " + clicked);
			// return;
			// //var clicked = parseInt($(this).attr("tabindex"));
			// if(typeof clicked !== "undefined"){
				// var current = $('#shapesAccordion').accordion('option', 'active');
				// alert("curr: " + current + " , " + clicked);
				// if($(this).hasClass('ui-accordion-header-active')){
					// $("#shapesAccordion").accordion({ active: false });
				// }
				// else {
					// $("#shapesAccordion").accordion({ active: clicked });
				// }
			// }
		// });
		// $("")
			
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
		$('#reset').click();
		icoMode = true;
		_AddIcosahedronOnClick();
		addShape();
		addShape(2); //Both the tetra and Icosahedron are added		
		toggleIcoMode();
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
		//customPolygonDialog.dialog("close");
		var numSides = parseInt($("#numSides").val());
		if(numSides < 3 || numSides > 99){
			alert("Invalid number of sides");
			return;
		}
		_AddPolygonOnClick(numSides);
		addShape();
	});
	
	$("#submitMeshBtn").click(function() {

		if($("#meshFileUpload")[0].files != null && $("#meshFileUpload")[0].files.length > 0){
			
			var fileName = $("#meshFileUpload")[0].files[0].name;
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
			reader.readAsText ($("#meshFileUpload")[0].files[0]);
		}
		else {
			alert("No");
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
			pickedShapeIndex = -1;
			var myNode = document.getElementById("shapesAccordion");
			while (myNode.firstChild) {
				myNode.removeChild(myNode.firstChild);
			}
			icoMode = false;
			toggleIcoMode();
		}
		else {
			//Clicked stop:
			if(isImpulseMode){
				isImpulseMode = false;
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
	});
	
	$("#sceneShapesHideBtn").click(function() {
		$("#rightSidePanel").hide("slide", { direction: "right" }, 1000);
		$.when($("#sceneShapesShowBtnDiv").show("slide", { direction: "right" }, 1000)).then(function() {
			sceneShapesOpen = false;
			window.dispatchEvent(new Event('resize'));
		});
	});
	
	$("#saveSceneBtn").click(function() {
		// var charPtr = _getSceneCSV();
		// var csv = Pointer_stringify(charPtr);
		var csv = Module.ccall('getSceneCSV', // name of C function
        'string');
		console.log(csv);
		
		var csvRows = csv.split(/\r|\n/);
		console.log(csvRows);
		var csvString = csvRows.join("\r\n");
		
		var a         = document.createElement('a');
		a.href        = 'data:attachment/csv,' +  encodeURIComponent(csvString);
		a.target      = '_blank';
		a.download    = 'multicollide_config.csv';

		document.body.appendChild(a);
		a.click();
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
	
	$("#canvas").mouseleave(function() {
		_deselectShape();
		clearSelection();
	});
	
	$("#canvas").mouseenter(function() {
		clearSelection();
	});
		  
}

function clearSelection() {
    if ( document.selection ) {
        document.selection.empty();
    } else if ( window.getSelection ) {
        window.getSelection().removeAllRanges();
    }
}

var jacobs_frame = false;
var sceneShapesOpen = true;
var icoMode = false;

function multicollide_update() {
	if(_isNewData()){
	  updateShapeData();
	}
	if(_isImpulseMode() != isImpulseMode){
	  isImpulseMode = _isImpulseMode();
	  toggleImpulseControls();
	}
	var currentlyPickedIndex = _getPickedShapeIndex();
	if(pickedShapeIndex != currentlyPickedIndex && currentlyPickedIndex != -1 && sceneShapesOpen){
		pickedShapeIndex = currentlyPickedIndex;
		openPickedShapeInfo();
	}
}
 
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
		
		//Select orientation tab on scene shapes panel
		$( ".tabs" ).tabs({ active: 1 }); //0 = PC, 1 = Orientation, 2 = Velocities
 }
 
  document.getElementById('rotate').onclick = function() {
		_RotateOnClick();
		
		//Select orientation tab on scene shapes panel
		$( ".tabs" ).tabs({ active: 1 }); //0 = PC, 1 = Orientation, 2 = Velocities
 }
 
  document.getElementById('scale').onclick = function() {
		_ScaleOnClick();
		
		//Select orientation tab on scene shapes panel
		$( ".tabs" ).tabs({ active: 0 }); //0 = PC, 1 = Orientation, 2 = Velocities
 }
 
  document.getElementById('velocity').onclick = function() {
		_VelocityOnClick();
		
		//Select orientation tab on scene shapes panel
		$( ".tabs" ).tabs({ active: 2 }); //0 = PC, 1 = Orientation, 2 = Velocities
 }
 
 document.getElementById('angularVelocity').onclick = function() {
		_AngularVelocityOnClick();
		
		//Select orientation tab on scene shapes panel
		$( ".tabs" ).tabs({ active: 2 }); //0 = PC, 1 = Orientation, 2 = Velocities
 }
 
 function getShapeData(i){
	 var data = [];

	 if(jacobs_frame){
		data["positionx"] = _getShapePositionX(i);
		data["positiony"] = _getShapePositionY(i);
		data["positionz"] = _getShapePositionZ(i);
		data["velocityx"] = _getShapeVelocityX(i);
		data["velocityy"] = _getShapeVelocityY(i);
		data["velocityz"] = _getShapeVelocityZ(i);
		data["mass"] = _getShapeMass(i);
		data["scale"] = _getShapeScale(i);
		//data["friction"] = _getShapeFriction(i);
		data["angularvelocityx"] = _getShapeAngularVelocityX(i);
		data["angularvelocityy"] = _getShapeAngularVelocityY(i);
		data["angularvelocityz"] = _getShapeAngularVelocityZ(i);
		data["angularvelocityspeed"] = _getShapeAngularVelocitySpeed(i);
		//data["restitution"] = _getShapeRestitution(i);
		
		data["rotationAxisX"] = _getShapeRotationAxisX(i);
		data["rotationAxisY"] = _getShapeRotationAxisY(i);
		data["rotationAxisZ"] = _getShapeRotationAxisZ(i);
		data["rotationAngle"] = _getShapeRotationAngle(i);
	 }
	 else {
		 data["positionx"] = _getShapePositionX(i);
		data["positiony"] = -1.0 * _getShapePositionZ(i);
		data["positionz"] = _getShapePositionY(i);
		data["velocityx"] = _getShapeVelocityX(i);
		data["velocityy"] = -1.0 * _getShapeVelocityZ(i);
		data["velocityz"] = _getShapeVelocityY(i);
		data["mass"] = _getShapeMass(i);
		data["scale"] = _getShapeScale(i);
		//data["friction"] = _getShapeFriction(i);
		data["angularvelocityx"] = _getShapeAngularVelocityX(i);
		data["angularvelocityy"] = -1.0 * _getShapeAngularVelocityZ(i);
		data["angularvelocityz"] = _getShapeAngularVelocityY(i);
		data["angularvelocityspeed"] = _getShapeAngularVelocitySpeed(i);
		//data["restitution"] = _getShapeRestitution(i);
		
		data["rotationAxisX"] = _getShapeRotationAxisX(i);
		data["rotationAxisY"] = -1.0 * _getShapeRotationAxisZ(i);
		data["rotationAxisZ"] = _getShapeRotationAxisY(i);
		data["rotationAngle"] = _getShapeRotationAngle(i);

	 }
	
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
 
 var pickedShapeIndex = -1;
 
 var showImpulseGraph = false;
 var showVelocityGraph = false;
 var isImpulseMode = false;
 
 var impulsePts = [];
 var velocityPts = [];
 
 function toggleImpulseControls() {
	 console.log("Toggling impulse controls");
	if(!isImpulseMode) {
        //$('#impulseControls').hide();//"slide", { direction: "right" }, 1000);
		//console.log("width: " + $('#impulseControls').width());
		//window.dispatchEvent(new Event('resize'));
		
		// $.when($("#graphsDiv").hide("slide", { direction: "right" }, 1000)).then(function() {
			// $("#sceneShapesDiv").show("slide", { direction: "right" }, 1000);
			
		// });
		
		$("#impulseContinueBtn").click();
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
		while(showVelocityGraph && _isMoreVelocityPts()){
			velocityPts.push(_getNextVelocityPtX());
			velocityPts.push(_getNextVelocityPtY());
			velocityPts.push(_getNextVelocityPtZ());
		}
		
		impulsePts = [];
		while(showImpulseGraph && _isMoreImpulsePts()){
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
		var endOfCompression = _getImpactResultsEndOfCompression();
		
		var IendX = round3(_getImpactResultsIendX());
		var IendY = round3(_getImpactResultsIendY());
		var IendZ = round3(_getImpactResultsIendZ());
		
		if(!jacobs_frame){
			var temp = v1EndY;
			v1EndY = -1.0 * v1EndZ;
			v1EndZ = temp;
			
			temp = v2EndY;
			v2EndY = -1.0 * v2EndZ;
			v2EndZ = temp;
			
			temp = w1EndY;
			w1EndY = -1.0 * w1EndZ;
			w1EndZ = temp;
			
			temp = w2EndY;
			w2EndY = -1.0 * w2EndZ;
			w2EndZ = temp;
			
			temp = IendY;
			IendY = -1.0 * IendZ;
			IendZ = temp;
		}
		
		
		//TODO if 2D don't output z's
		var html = "<h4 id='impactVisualizations'>A Collision Occurred</h4>" + 
			"<div id='graphsContainer' height='520px'>";
			
			if(showImpulseGraph){
				html += "<p class='graphLabel'>Impulse Accumulation</p>" + 
				"<div id='graph1'  height='230px' class='centered-div'></div><br><br>";
			}
			
			if(showVelocityGraph){
				html += "<p class='graphLabel'>Sliding Velocity Curve (Hodograph)</p>" +
				"<canvas id='graph2' height='240px' class='centered-div'></canvas>";
			}
				
				
				
			html += 
				"<br><br><b><i>Post-impact values: </i></b><br>" +
				"<table class='postImpactTable'><tr><td class='postImpactCell'></td><td class='postImpactCell'>" + Pointer_stringify(_getContactShape1Name()) + "</td><td class='postImpactCell'>" + Pointer_stringify(_getContactShape2Name()) + "</td></tr>" + 
					"<tr><td class='postImpactCell'><span class='vectorNotation'>v+</span></td><td class='postImpactCell'>";
					if(!using2D){
						html += "<div class='vectorDiv'>" + getHtmlVector(v1EndX, v1EndY, v1EndZ) + "</div>";
					}
					else {
						html += "<div class='vectorDiv'>" + getHtmlVector(v1EndX, v1EndY) + "</div>";
					}
					html += "</td><td class='postImpactCell'>";
					if(!using2D){
						html += "<div class='vectorDiv'>" + getHtmlVector(v2EndX, v2EndY, v2EndZ) + "</div>";
					}
					else {
						html += "<div class='vectorDiv'>" + getHtmlVector(v2EndX, v2EndY) + "</div>";
					}
					html += "</td></tr>" +
					"<tr><td class='postImpactCell'><span class='vectorNotation'>w+</span></td><td class='postImpactCell'>" 
					if(!using2D){
						html += "<div class='vectorDiv'>" + getHtmlVector(w1EndX, w1EndY, w1EndZ) + "</div>";
					}
					else {
						html += w1EndX;
					}
					html += "</td><td class='postImpactCell'>";
					if(!using2D){
						html += "<div class='vectorDiv'>" + getHtmlVector(w2EndX, w2EndY, w2EndZ) + "</div>";
					}
					else {
						html += w2EndX;
					}
					html += "</td></tr>";
				html += "</table>";
				
				
				html += "<br><br><b><i>Impulse information: </i></b><br><table class='postImpactTable'>" +
					"<tr><td class='postImpactCell'>Total Impulse</td><td class='postImpactCell'><div class='vectorDiv'>" + getHtmlVector(IendX, IendY, IendZ) + "</div></td></tr>" +
								
				"<tr><td class='postImpactCell'>End of Sliding</td><td class='postImpactCell'>" + endOfSliding + "</td></tr>" +
				"<tr><td class='postImpactCell'>End of Compression</td><td class='postImpactCell'>" + endOfCompression + "</td></tr></table>";
				
				html += "<br><br><div class='centered-div'><input type='button' class='ui-button ui-widget ui-corner-all' id='impulseContinueBtn' value='Continue'>" +
				"<input type='button' class='ui-button ui-widget ui-corner-all' id='impulseRestartBtn' value='Restart'></div>" +
			"</div>";
		
		var accord = $('#shapesAccordion');
		accord.append(html);
		accord.accordion("refresh");
		$(".svg-containger").attr('margin', '0 auto');
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
			if(showImpulseGraph) {
				$("#graph1").empty();
			}
			if(showVelocityGraph){
				$("#graph2").empty();
			}
			$("#graph2").empty();
			if(showImpulseGraph){
				drawImpulse(impulsePts);
			}
			if(showVelocityGraph) {
				drawVelocity(velocityPts);
			}
		});
		
		var showGraph = function() {
			var indexToOpen = accord.children('div').length - 1;
			accord.accordion('option', 'active', indexToOpen);
			if(showImpulseGraph){
				drawImpulse(impulsePts, endOfCompression, endOfSliding);
			}
			if(showVelocityGraph) {
				drawVelocity(velocityPts, endOfCompression, endOfSliding);
			}
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
 
 function openPickedShapeInfo() {
	 $('#shapesAccordion').accordion('option', 'active', pickedShapeIndex);
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
 
 function getHtmlMatrix(xx, xy, xz, yx, yy, yz, zx, zy, zz){
	 return "<table class='matrix'>" + 
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
 }
 
  function getHtmlVector(x, y, z){
	 var html = "<table class='matrix'>" + 
				"<tr>" + 
					"<td>" + x + "</td></tr>" + 
					"<tr><td>" + y + "</td></tr>";
					if(typeof z !== undefined)
						html += "<tr><td>" + z + "</td></tr>";
			html +=	"</table>";
	return html;
 }
 
 function calculateAngularVelocitySpeed(index, sectionId) {
	var speed = _getShapeAngularVelocitySpeed(index);
	console.log(speed);
	 $('#' + sectionId + '.angularvelocityspeed').text(speed);
 }
 
 function calculateNormalizedRotAxis(index, sectionId) {
	 var rotX = _getShapeRotationAxisX(index);
	 var rotY = _getShapeRotationAxisY(index);
	 var rotZ = _getShapeRotationAxisZ(index);
	 console.log(index);
	 console.log('' + rotX + ',' + rotY + ',' + rotZ);
	 var mag = Math.sqrt(rotX*rotX + rotY*rotY + rotZ*rotZ);
	 console.log(mag);
	 $('#' + sectionId + '.normalizedRotAxisX').text(rotX/mag);
 }
 
 var using2D = false;
 
 //Toggles between 2D/3D interface mode
 function initDimension(simMode){
	 if(simMode === '3D'){
		_use2DShapes(false);
		$("#2DaddShapeMenu").hide();
		$("#3DaddShapeMenu").show();
		
		$("#cameraDomToHide").show();
		$(".hideIn2D").show();
		$("#angularVelocity").show();
		
		using2D = false;
	 }
	 else {
		_use2DShapes(true);
		$("#2DaddShapeMenu").show();
		$("#3DaddShapeMenu").hide();
		
		//$("#cameraDomToHide").hide();
		$(".hideIn2D").hide();
		//$("#angularVelocity").hide();
		
		using2D = true;
	 }
	 
	 $("#reset").click();
 }
 
 /*
	0 - sphere
	1 - cube 
	2 - ellipsoid
	3 - superquadric
	4 - custom superquad, a1, a2, a3, e1, e2
	5 - mesh - all data?
	6 - circle  
	7 - triangle
	8 - square
	9 - custom poly - all vertices, x1, y1, x2, y2,...
	10 - icosahedron
	11 - tetrahedron
	*/
 function parseSceneCSV(fileContents) {
	 
	 $("#reset").click();
	 
	var allRows = fileContents.split(/\r?\n|\r/);
	
	var foundFirstShapeLine = false;
	var foundSceneInfo = false;
	
	for(var row = 0; row < allRows.length; row++) {
		var rowCells = allRows[row].split(',');
		if(rowCells.length == 0){
			continue;
		}
		if(!foundSceneInfo){
			var isUsing2D = parseInt(rowCells[0]);
			var mu = parseFloat(rowCells[1]);
			var e = parseFloat(rowCells[2]);
			
			var simMode = '3D';
			if(isUsing2D){
				simMode = '2D';
			}
			initDimension(simMode);
			
			_setShapeFriction(mu);
			_setShapeRestitution(e);
			foundSceneInfo = true;
		}
		else {
			if(!foundFirstShapeLine){
				var shapeId = parseInt(rowCells[0]);
				switch(shapeId) {
					case 0:
						$('#addSphereBtn').click();
						break;
					case 1:
						$('#addCubeBtn').click();
						break;
					case 2:
						$('#addEllipsoidBtn').click();
						break;
					case 3:
						$('#addSuperquadricBtn').click();
						break;
					case 4:
						var a1 = parseFloat(rowCells[1]);
						var a2 = parseFloat(rowCells[2]);
						var a3 = parseFloat(rowCells[3]);
						var e1 = parseFloat(rowCells[4]);
						var e2 = parseFloat(rowCells[5]);
						_createCustomSuperquadric(a1, a2, a3, e1, e2);
						addShape();
						break;
					case 5:
						//TODO mesh 
						break;
					case 6:
						$('#addCircleBtn').click();
						break;
					case 7:
						$('#addTriangleBtn').click();
						break;
					case 8:
						$('#addSquareBtn').click();
						break;
					case 9:
						_createNewCustomPolygon();
						for(var cell=1; cell<rowCells.length; cell+=2){
							var x = parseFloat(rowCells[cell]);
							var y = parseFloat(rowCells[cell+1]);
							_addCustomPolygonVertex(x, y);
						}
						_doneCreatingCustomPolygon();
						addShape();
						break;
				}
				foundFirstShapeLine = true;
			}
			else {
				/*
					if(dataElement === "positionx") {
					_setShapePositionX(index, value);
				} else if(dataElement === "positiony"){
					_setShapePositionZ(index, -1.0 * value);
				} else if(dataElement === "positionz"){
					_setShapePositionY(index, value);
				*/
				
				var curShapeIndex = _getNumShapes() -1;
				foundFirstShapeLine = false;
				var transX = parseFloat(rowCells[0]);
				var transY = parseFloat(rowCells[1]);
				var transZ = parseFloat(rowCells[2]);
				
				_setShapePositionX(curShapeIndex, transX);
				_setShapePositionY(curShapeIndex, transY);
				_setShapePositionZ(curShapeIndex, transZ);
				
				var velX = parseFloat(rowCells[3]);
				var velY = parseFloat(rowCells[4]);
				var velZ = parseFloat(rowCells[5]);
				
				_setShapeVelocityX(curShapeIndex, velX);
				_setShapeVelocityY(curShapeIndex, velY);
				_setShapeVelocityZ(curShapeIndex, velZ);
				
				var rotAngle = parseFloat(rowCells[6]);
				var rotAxisX = parseFloat(rowCells[7]);
				var rotAxisY = parseFloat(rowCells[8]);
				var rotAxisZ = parseFloat(rowCells[9]);
				
				//_setShapeRotationAngle(curShapeIndex, rotAngle);
				//_setShapeRotationAxisX(curShapeIndex, rotAxisX);
				//_setShapeRotationAxisY(curShapeIndex, rotAxisY);
				//_setShapeRotationAxisZ(curShapeIndex, rotAxisZ);
				
				var angVelX = parseFloat(rowCells[10]);
				var angVelY = parseFloat(rowCells[11]);
				var angVelZ = parseFloat(rowCells[12]);
				
				_setShapeAngularVelocityX(curShapeIndex, angVelX);
				_setShapeAngularVelocityY(curShapeIndex, angVelY);
				_setShapeAngularVelocityZ(curShapeIndex, angVelZ);
				
				var scaling = parseFloat(rowCells[13]);
				var mass = parseFloat(rowCells[14]);
				_setShapeScale(curShapeIndex, scaling);
				_setShapeMass(curShapeIndex, mass);
			}
		}
	}
 }
 
 function toggleIcoMode() {
	//$(".icoDisable").prop("disabled",icoMode);
	
	$(".icoDisable").button({
		disabled: icoMode
	});
	
	// $(".icoDisable").hover(function(){
		
	// });
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
	
	showVelocityGraph = (localStorage.getItem('showVelocity') === 'true');
	$("#showVelocity").prop('checked', showVelocityGraph);
	checkIfShouldShowGraphs();
	
	//mu & e:
	$("#friction").val(_getShapeFriction());
	$("#restitution").val(_getShapeRestitution());
 }
 
 function roundNumber(num){
	 return Math.round(num * 100) / 100
 }
 
 function initSettingsStorage() {
	 localStorage.setItem('simMode', '3D');
	 localStorage.setItem('freefly', true);
	 localStorage.setItem('rotationDisplay', 'rotationAxis');
	 localStorage.setItem('showImpulse', true);
	 localStorage.setItem('showVelocity', true);
 }