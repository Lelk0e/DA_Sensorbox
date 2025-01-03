let x_axe_length = 0;
let y_axe_length = 0;

function apply_button(){ //applying the filter options on the graph

}

function update(){

}

function setTime() { //from phillip
    // Get the current time from the client's device
    var currentTime = new Date();
    var hour = currentTime.getHours();
    var minute = currentTime.getMinutes();
    var second = currentTime.getSeconds();

    // Send the time to the ESP32
    var xhr = new XMLHttpRequest();
    xhr.open(
      "GET",
      "/set-time?hour=" +
        hour +
        "&minute=" +
        minute +
        "&second=" +
        second,
      true
    );
    xhr.onload = function () {
      alert("Time set successfully: " + xhr.responseText);
    };
    xhr.send();
}

function canvas_setting(){
    //here i will draw a xy Graph with legends
    const canvas = document.getElementById('graph');
    const ctx = canvas.getContext("2d"); //getting the the features of the canvas, so i can promptly edit it AKA the context ... ctx

    //getting the height and width
    const width = canvas.width;
    const height = canvas.height;
    
    //clearing the canvas
    ctx.clearRect(0, 0, width, height);

    create_graph_xy();
}

function create_graph_xy(){
    const canvas = document.getElementById('graph');
    const ctx = canvas.getContext("2d"); //getting the the features of the canvas, so i can promptly edit it AKA the context ... ctx

    //getting the height and width
    const width = canvas.width;
    const height = canvas.height;

    //fixing the center point, so i can draw lines with negative coordinates
    ctx.translate(width*0.2, height*0.9); //center point for all objects, where i will start to draw
    //this point is ABSOlUTE, i do not have to change it or add it again!

    //giving the lines a color and stroke width
    ctx.lineWidth = 2;
    ctx.strokeStyle = '#000000';

    //beginnig to draw main lines
    //y line
    ctx.beginPath(); 
    ctx.moveTo(0, 0); //0, 0 because we have a starting point ... translate(...)
    ctx.lineTo(0, -height*0.8);
    ctx.stroke(); //drawing the elements
    //x line
    ctx.beginPath();
    ctx.moveTo(0, 0);
    ctx.lineTo(width*0.7, 0);
    ctx.stroke(); //drawing the elements

    //beginning to draw arrows of the main lines
    //arrow y
    ctx.beginPath();
    ctx.moveTo(0, -height*0.8);
    ctx.lineTo(width*0.01, -height*0.75);
    ctx.lineTo(-width*0.01, -height*0.75);
    ctx.lineTo(0, -height*0.8);
    ctx.stroke();
    y_axe_length = height*0.75; //saving the length for later

    //arrow x
    ctx.beginPath();
    ctx.moveTo(width*0.7, 0);
    ctx.lineTo(width*0.65, -height*0.02);
    ctx.lineTo(width*0.65, height*0.02);
    ctx.lineTo(width*0.7, 0);
    ctx.stroke();
    x_axe_length = width*0.65; //saving the length for later
                               //In addition, it depends on the arrow, because the lines of axe (x,y), 
                               //would cross over the arrow, this would look ugly

    //here i am sending httrequest, so our esp gets the current time
    //setTime();
}

function time_setting(){   
    //reading zone ... getting the element and reading the value, which currently used e.g. sellected
    const zone = document.getElementById("box_time").value;
    let lapse = 0; //whole line portions for the x axe, e.g. a day has 24 --> 24 hours, meaning 24 lines
    let unit = 0;  //for each time lapse

    //here i will draw a xy Graph with legends
    const canvas = document.getElementById('graph');
    const ctx = canvas.getContext("2d"); //getting the the features of the canvas, so i can promptly edit it AKA the context ... ctx
    
    //getting the height and width
    const width = canvas.width;
    const height = canvas.height;

    switch (zone) {
        case "Today":
            ctx.clearRect(0, 1, x_axe_length-1, height*0.01); //clearing the axe, so it is cleared
                                                              //we are beginning at 1, because line with
            measure_text_x_axe_and_delete(); //deleting the text
            lapse = 24 + 1; //1, because of 0 time
            unit = x_axe_length / lapse;
            for (let i = 0; i < lapse; i++) {    
                ctx.beginPath();
                ctx.moveTo(i * unit, 0);
                ctx.lineTo(i*unit, height*0.01);
                ctx.lineWidth = 2; //this one
                ctx.strokeStyle = '#000000';
                ctx.stroke();
            } 
            ctx.font = "20px serif";
            ctx.fillText("x in hours", width*0.705,1);  
        break;
        
        case "Yesterday":
            ctx.clearRect(0, 1, x_axe_length-1, height*0.01); //clearing the axe, so it is cleared
                                                              //we are beginning at 1, because line with
            measure_text_x_axe_and_delete(); //deleting the text
            lapse = 24 + 1; //1, because of 0 time
            unit = x_axe_length / lapse; //for each time lapse
            for (let i = 0; i < lapse; i++) {    
                ctx.beginPath();
                ctx.moveTo(i * unit, 0);
                ctx.lineTo(i*unit, height*0.01);
                ctx.lineWidth = 2;
                ctx.strokeStyle = '#000000';
                ctx.stroke();
            }
            ctx.font = "20px serif";
            ctx.fillText("x in hours", width*0.705,1);
        break;

        case "This Week":
            ctx.clearRect(0, 1, x_axe_length-1, height*0.01); //clearing the axe, so it is cleared
                                                              //we are beginning at 1, because line with
            measure_text_x_axe_and_delete(); //deleting the text
            lapse = 7 + 1; //1, because of 0 time
            unit = x_axe_length / lapse; //for each time lapse
            for (let i = 0; i < lapse; i++) {    
                ctx.beginPath();
                ctx.moveTo(i * unit, 0);
                ctx.lineTo(i*unit, height*0.01);
                ctx.lineWidth = 2;
                ctx.strokeStyle = '#000000';
                ctx.stroke();
            }
            ctx.font = "20px serif";
            ctx.fillText("x in days", width*0.705,1);
        break;

        case "Last Week":
            ctx.clearRect(0, 1, x_axe_length-1, height*0.01); //clearing the axe, so it is cleared
                                                              //we are beginning at 1, because line with
            measure_text_x_axe_and_delete(); //deleting the text
            lapse = 7 + 1; //1, because of 0 time
            unit = x_axe_length / lapse; //for each time lapse
            for (let i = 0; i < lapse; i++) {    
                ctx.beginPath();
                ctx.moveTo(i * unit, 0);
                ctx.lineTo(i*unit, height*0.01);
                ctx.lineWidth = 2;
                ctx.strokeStyle = '#000000';
                ctx.stroke();
            }
            ctx.font = "20px serif";
            ctx.fillText("x in days", width*0.705,1);
        break;

        case "This Month":
            ctx.clearRect(0, 1, x_axe_length-1, height*0.01); //clearing the axe, so it is cleared
                                                              //we are beginning at 1, because line with
            measure_text_x_axe_and_delete(); //deleting the text
            lapse = 30 + 1; //1, because of 0 time
            unit = x_axe_length / lapse; //for each time lapse
            for (let i = 0; i < lapse; i++) {    
                ctx.beginPath();
                ctx.moveTo(i * unit, 0);
                ctx.lineTo(i*unit, height*0.01);
                ctx.lineWidth = 2;
                ctx.strokeStyle = '#000000';
                ctx.stroke();
            }
            ctx.font = "20px serif";
            ctx.fillText("x in days", width*0.705,1);
        break;

        case "Last Month":
            ctx.clearRect(0, 1, x_axe_length-1, height*0.01); //clearing the axe, so it is cleared
                                                              //we are beginning at 1, because line with
            measure_text_x_axe_and_delete(); //deleting the text
            lapse = 30 + 1; //1, because of 0 time
            unit = x_axe_length / lapse; //for each time lapse
            for (let i = 0; i < lapse; i++) {    
                ctx.beginPath();
                ctx.moveTo(i * unit, 0);
                ctx.lineTo(i*unit, height*0.01);
                ctx.lineWidth = 2;
                ctx.strokeStyle = '#000000';
                ctx.stroke();
            }
            ctx.font = "20px serif";
            ctx.fillText("x in days", width*0.705,1);
        break;

        case "This Year":
            ctx.clearRect(0, 1, x_axe_length-1, height*0.01); //clearing the axe, so it is cleared
                                                              //we are beginning at 1, because line with
            measure_text_x_axe_and_delete(); //deleting the text
            lapse = 365 + 1; //1, because of 0 time
            unit = x_axe_length / lapse; //for each time lapse
            for (let i = 0; i < lapse; i++) {    
                ctx.beginPath();
                ctx.moveTo(i * unit, 0);
                ctx.lineTo(i*unit, height*0.01);
                ctx.lineWidth = 0.5;
                ctx.strokeStyle = '#000000';
                ctx.stroke();
            }
            ctx.font = "20px serif";
            ctx.fillText("x in days", width*0.705,1);
        break;

        case "Last Year":
            ctx.clearRect(0, 1, x_axe_length-1, height*0.01); //clearing the axe, so it is cleared
                                                              //we are beginning at 1, because line with
            measure_text_x_axe_and_delete(); //deleting the text
            lapse = 365 + 1; //1, because of 0 time
            unit = x_axe_length / lapse; //for each time lapse
            for (let i = 0; i < lapse; i++) {    
                ctx.beginPath();
                ctx.moveTo(i * unit, 0);
                ctx.lineTo(i*unit, height*0.01);
                ctx.lineWidth = 0.5;
                ctx.strokeStyle = '#000000';
                ctx.stroke();
            }
            ctx.font = "20px serif";
            ctx.fillText("x in days", width*0.705,1);
        break;
    }
}

function measure_text_x_axe_and_delete(){
    const canvas = document.getElementById('graph');
    const ctx = canvas.getContext("2d"); //getting the the features of the canvas, so i can promptly edit it AKA the context ... ctx
    
    //getting the width
    const width = canvas.width;

    //getting the measurments
    var x_days_lenth = ctx.measureText("x in days").width;
    var x_hours_lenth = ctx.measureText("x in hours").width;

    //now deleting
    ctx.clearRect(width*0.705, 1, x_days_lenth, -20); //20px, we have to do -, because it's allignment is up
    ctx.clearRect(width*0.705, 1, x_days_lenth, 20); //double insurance, sometimes it deletes almost everything, but not all the text
    ctx.clearRect(width*0.705, 1, x_hours_lenth, -20);
    ctx.clearRect(width*0.705, 1, x_hours_lenth, 20); //same here

}

function sensor_setting(){
    //the same like setting_time()
    const sensor = document.getElementById("box_sens").value;

    switch (sensor) {
        case "Temperature":
            
        break;

        case "Airmoisure":
            
        break;

        case "Airpressure":
            
        break;

        case "Gas":
            
        break;
    }
}

function HTTP_SET(){
    
}

