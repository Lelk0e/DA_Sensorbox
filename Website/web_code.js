

function setup(){

}

function apply_button(){ //applying the filter options on the graph

}

function update(){

}

function canvas_setting(){
    //here i will draw a xy Graph with legends
    const canvas = document.getElementById('graph');
    const ctx = canvas.getContext("2d"); //getting the the features of the canvas, so i can promptly edit it AKA the context ... ctx

    //getting the height and width
    const width = canvas.width;
    const height = canvas.height;
    
    //clearing the canvas
    //canvas.clearRect(0,0, width, height);

    //fixing the center point, so i can draw lines with negative coordinates
    ctx.translate(width*0.2, height*0.9); //center point for all objects, where i will start to draw

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
    //arrow x
    ctx.beginPath();
    ctx.moveTo(width*0.7, 0);
    ctx.lineTo(width*0.65, -height*0.02);
    ctx.lineTo(width*0.65, height*0.02);
    ctx.lineTo(width*0.7, 0);
    ctx.stroke();
}

function time_setting(){
    //reading zone ... getting the element and reading the value, which currently used e.g. sellected
    const zone = document.getElementById("box_time").value;
        
    switch (zone) {
        case "Today":
            
        break;
        
        case "Yesterday":
            
        break;

        case "This Week":
            
        break;

        case "Last Week":
            
        break;

        case "This Month":
            
        break;

        case "Last Month":
            
        break;

        case "This Year":
            
        break;

        case "Last Year":
            
        break;
    }
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

function HTTP_GET(){

}

function HTTP_SET(){
    
}