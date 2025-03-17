//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&//
//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&//
//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&//
//-----------------------------------------------------------------------------//
//for axe naming
let x_whole_axe_length = 0;
let y_whole_axe_length = 0; 
//important for some naming and scaling the graph
let x_axe_length = 0;
let y_axe_length = 0;
//important for the scaling naming of each axes
let y_point_length = 0;
let x_point_length = 0;
//-----------------------------------------------------------------------------//
//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&//
//-----------------------------------------------------------------------------//
//sensor max values
const bme_max_value = 1100; //1100hPa
const hpp_max_value = 100; //100% is the max value
const ozon_max_value = 100; //100ppm is the max value
const temp_max_value = 250; //250°C is the max value
//-----------------------------------------------------------------------------//
//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&//
//-----------------------------------------------------------------------------//
//coordnates for the drawn data-objects
//canvas bme data & time
let bme_canvas_x = []; //for those, who will be drawn on the canvas, so that I can delete them without any problems
let bme_canvas_y = [];
//canvas hpp data & time
let hpp_canvas_x = []; 
let hpp_canvas_y = [];
//canvas ozon data & time
let ozon_canvas_x = []; 
let ozon_canvas_y = [];
//canvas temp data & time
let temp_canvas_x = []; 
let temp_canvas_y = [];
//-----------------------------------------------------------------------------//
//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&//
//-----------------------------------------------------------------------------//
//variables for occuring data flow, like bme, hpp, etc.
let data_bme = "";
let data_hpp = "";
let data_ozon = "";
let data_temp = "";
//the same, but without any symbols
let data_bme_no_sym = "";
let data_hpp_no_sym = "";
let data_ozon_no_sym = "";
let data_temp_no_sym = "";
//split sensor info
let bme_split = [];
let hpp_split = [];
let ozon_split = [];
let temp_split = [];
//data and time of the corrosponding sensor
//bme
let bme_time = [];
let bme_data = [];
//hpp
let hpp_time = [];
let hpp_data = [];
//ozon
let ozon_time = [];
let ozon_data = [];
//temp
let temp_time = [];
let temp_data = [];
//-----------------------------------------------------------------------------//
//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&//
//-----------------------------------------------------------------------------//
//for measure off/on button
let count_button_off_on = 0;

//after deleting the graph, data should be shown/calculated --> without it, graph data will be shown in a wrong pattern.
let u_can_calculate_now = false;

//alerting me as a programer, if http-set was read/saved pefectly 
let read_data_bool = false;
let save_data_bool = false;
//-----------------------------------------------------------------------------//
//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&//
//-----------------------------------------------------------------------------//
//bme
let time_max_bme = 0; //time max and divided time of the general time division
let time_min_bme = 0;
let time_division_bme =  0; //general time division ... number

let data_max_bme = 0; //max data for the bme
let data_min_bme = 0; //lowest data for the bme
let data_division_bme = 0; //division factor for the coordinates and drawing

//hpp
let time_max_hpp = 0; //time max and divided time of the general time division
let time_min_hpp = 0;
let time_division_hpp = 0; //general time division ... number

let data_max_hpp = 0;
let data_min_hpp = 0;
let data_division_hpp = 0;

//ozon
let time_max_ozon = 0; //time max and divided time of the general time division
let time_min_ozon = 0;
let time_division_ozon = 0; //general time division ... number

let data_max_ozon = 0;
let data_min_ozon = 0;
let data_division_ozon = 0;

//temp
let time_max_temp = 0; //time max and divided time of the general time division
let time_min_temp = 0;
let time_division_temp = 0; //general time division ... number

let data_max_temp = 0;
let data_min_temp = 0;
let data_division_temp = 0;

//important values for drawing --- FOR ALL SENSORS
let max_time_zone = 0;
let min_time_zone = 0;
let division_time_zone = 0;

let max_data_zone = 0;
let min_data_zone = 0;
let division_data_zone = 0;
//-----------------------------------------------------------------------------//
//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&//
//-----------------------------------------------------------------------------//
//esp realization/implementation-----------------------------------------------//
let sensor_mac_addresses_sensorbox = []; //will save all mac-addresses from the different sensorboxes in this list
let sensor_all_data_sensorbox = []; //4 different arrays will be in there: hpp, ozon, bme, temp
//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&//
//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&//
//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&//

function apply_button(){ //applying the filter options on the graph
    update();
}

function update(){ //changing the data, which shall be shown to the user

    HTTP_SAVE(); //saving sensor-data ---> first HTTP_SAVE, because there are some pieces of information, which could be changed anytime

    canvas_setting(); //reseting everything

    const sensor = document.getElementById("box_sens").value;
    if (u_can_calculate_now == true){ 
        switch (sensor) { //draw the selected sensor
            case "Temperature":
                draw_temp();
            break;
        
            case "Airmoisure":
                draw_hpp();
            break;
    
            case "Airpressure":
                draw_bme();
            break;
    
            case "Gas":
                draw_ozon();
            break;
        }
    } 
}

function draw_bme(){ //previously i saved my sensor data here ... yeah i changed that, and added it to HTTP_SAVE(), it's saver
    //read bme_data
    //save_drawing_data_for_bme();
    draw_general(bme_canvas_x, bme_canvas_y);
}

function draw_hpp(){
    //read hpp data
    //save_drawing_data_for_hpp();
    draw_general(hpp_canvas_x, hpp_canvas_y);
}

function draw_ozon(){
    //save_drawing_data_for_ozon();
    draw_general(ozon_canvas_x, ozon_canvas_y);
}

function draw_temp(){
    //save_drawing_data_for_temp();
    draw_general(temp_canvas_x, temp_canvas_y);
}

function draw_general(coord_canvas_sensor_x, coord_canvas_sensor_y){
    //here i will draw a xy Graph with legends
    const canvas = document.getElementById('graph');
    const ctx = canvas.getContext("2d"); //getting the features of the canvas, so i can promptly edit it AKA the context ... ctx

    if (coord_canvas_sensor_x.length == coord_canvas_sensor_y.length){ //double check, that coordinates are correct
        const draw_till_u_die = coord_canvas_sensor_x.length;

        //drawing the points --> SSSR Rank
        for (let i = 0; i < draw_till_u_die; i++) { 
            ctx.beginPath();
            ctx.fillStyle = "pink";
            //console.log(Math.PI); //lets see if pi-pi is working xD --> well, that's a insider joke.
            ctx.ellipse(coord_canvas_sensor_x[i], -coord_canvas_sensor_y[i], 3, 3, 0, 0, 2 * Math.PI, false);
            ctx.stroke();
        }

        //Line creating
        ctx.beginPath();
        ctx.fillStyle = "darkblue";
        ctx.moveTo(coord_canvas_sensor_x[0], -coord_canvas_sensor_y[0]);

        for (let i = 0; i < coord_canvas_sensor_x.length; i++) { //drawing the lines --> same rank
            ctx.lineTo(coord_canvas_sensor_x[i], -coord_canvas_sensor_y[i]);
        }
        ctx.stroke();

        ctx.fillStyle = "black";
    }
}

function setTime() { //from phillip, some parts of it is from me, but it was kinda changed
    var check = false;
    
    try{
        // Get the current time from the client's device
        var currentTime = new Date();
        var date = currentTime.getDate(); //thats new, because we wanted to add dates to our time send routine
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
              second +
              "&date=" +
              date,
            true
        );
        xhr.onload = function () {
            alert("Time set successfully: " + xhr.responseText);
        };
        
        xhr.onerror = function(){ //on error function, cause those errors are only detectable through these methods
            console.warn("A error happened, duo to: " + xhr.status);
        };

        if (xhr.status == 200){ //200 meaning its okay and successfull
            xhr.send();
        }
    }
    catch(err){ //this catch wont necessary happen, because http request is kinda weird
      alert("It occurs a errors when sending the httprequest, please make sure, you are connected to your device!\n" + err);
    };
}

function measure_off_on_button(){
    count_button_off_on += 1;
    const button_off_on = document.getElementById("meas_ss_butt");
    
    if (count_button_off_on == 1){ //start measurement
        button_off_on.style.backgroundColor = "green";
        setTime();
    }
    else if (count_button_off_on == 2){ //stop measurement
        button_off_on.style.backgroundColor = "blue";
        setTime();
    }
    else{ //nothing will be happening here
        count_button_off_on = 0;
        button_off_on.style.backgroundColor = "dimgray";
        console.log("No Measurement is going on");
    }
}

function canvas_setting(){
    u_can_calculate_now = false;

    //here i will draw a xy Graph with legends
    const canvas = document.getElementById('graph');
    const ctx = canvas.getContext("2d"); //getting the features of the canvas, so i can promptly edit it AKA the context ... ctx

    //getting the height and width
    const width = canvas.width;
    const height = canvas.height;

    //clearing the canvas
    ctx.setTransform(1, 0, 0, 1, 0, 0); //works with a matrix --> a, b, c, d, e, f --> a horizonatal scale = 1 & d vertical scale = 1. The rest are default positions, which should be 0. Tada with that awesome command, i solved my biggest problem "Clearing my Canvas".
    ctx.clearRect(0, 0, width, height);

    create_graph_xy();

    //why, you ask. Very simple, because these methods will update the graph's x and y axe-lines, the combo boxes are sometimes bugged, and dont update at the beginning if you dont change the current item. I had several problems with that, maybe i will change this line of code in the future, but now it will stay like that --> yeah it will stay like that
    time_setting();
    sensor_setting();

    u_can_calculate_now = true;
}

function create_graph_xy(){
    const canvas = document.getElementById('graph');
    const ctx = canvas.getContext("2d"); //getting the features of the canvas, so i can promptly edit it AKA the context ... ctx
    
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
    y_whole_axe_length = height*0.8; //saving the whole length of the y axe
    //x line
    ctx.beginPath();
    ctx.moveTo(0, 0);
    ctx.lineTo(width*0.7, 0);
    ctx.stroke(); //drawing the elements
    x_whole_axe_length = width*0.7; //saving the whole length of the x axe

    //beginning to draw arrows of the main lines
    //arrow y
    ctx.beginPath();
    ctx.moveTo(0, -height*0.8);
    ctx.lineTo(width*0.01, -height*0.75);
    ctx.lineTo(-width*0.01, -height*0.75);
    ctx.lineTo(0, -height*0.8);
    ctx.stroke();
    y_axe_length = height*0.75; //saving the length for later
    y_point_length = y_axe_length*0.93; //important time data-showing
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
    x_point_length = x_axe_length*0.98; //important for the sensor data-showing
    setTime(); //here i am sending httrequest, so our esp gets the current time --> OFC this was not test properly, will be done later
}

function time_setting(){   
    //reading zone ... getting the element and reading the value, which was currently used e.g. sellected
    const zone = document.getElementById("box_time").value;
    let lapse = 0; //whole line portions for the x axe, e.g. a day has 24 --> 24 hours, meaning 24 lines
    let unit = 0;  //for each time lapse

    //here i will draw a xy Graph with legends
    const canvas = document.getElementById('graph');
    const ctx = canvas.getContext("2d"); //getting the features of the canvas, so i can promptly edit it AKA the context ... ctx
    
    //getting the height and width
    const width = canvas.width;
    const height = canvas.height;

    const x_axe_name_diff = width * 0.005; //dif ... difference
    const height_for_x_lapses = height*0.01;
    switch (zone) { //changes --> before it was x_axe_length - 1 in clearRect etc. --> But i found out that it is too much trouble, to implement it in that way all the time. So i gave the time and sensor axes each unique length, so that i have no trouble with line conflictions
        case "Current-Data-Time":

            const sensor = document.getElementById("box_sens").value;

            switch (sensor) {
                case "Temperature":
                    max_time_zone = time_max_temp;
                    min_time_zone = time_min_temp;
                    division_time_zone = time_division_temp;
                break;
            
                case "Airmoisure":
                    max_time_zone = time_max_hpp;
                    min_time_zone = time_min_hpp;
                    division_time_zone = time_division_hpp;
                break;

                case "Airpressure":
                    max_time_zone = time_max_bme;
                    min_time_zone = time_min_bme;
                    division_time_zone = time_division_bme;
                break;

                case "Gas":
                    max_time_zone = time_max_ozon;
                    min_time_zone = time_min_ozon;
                    division_time_zone = time_division_ozon;
                break;
            }

            if (read_data_bool == true && save_data_bool == true){

                lapse = max_time_zone - min_time_zone; //it can be a dot number, so yeaaah
                unit = division_time_zone; //for each time lapse --> x_point.length / max_time_zone 

                ctx.clearRect(-unit/2, 1, unit/2 + x_axe_length-1, height_for_x_lapses + 0.02*height + 12); //clearing the axe, so it is cleared
                                                              //we are beginning at 1, because of line width
                measure_text_x_axe_and_delete(); //deleting the text

                //because js is kinda bad, i have to do this method, to make the graph's line accuracy better
                const steps = lapse / 10;
                let steps_array = [];

                for (let i = 0; i <= 10; i++){ //i ignore the double numbers, and with that way, i can draw the lines perfectly
                    steps_array[i] = steps * i;
                }
                
                for (let i = 0; i <= steps_array.length; i++) { // i = 0.0, so we can make sure, that we work with double numbers --> yeeeeah, i changed that, because i am working with steps_array, i dont need to look at double numbers anymore
                    ctx.beginPath();
                    ctx.moveTo(steps_array[i] * unit, 0);
                    ctx.lineTo(steps_array[i] * unit, height_for_x_lapses);
                    ctx.lineWidth = 2;
                    ctx.strokeStyle = '#000000';
                    ctx.stroke();
                    //lapses naming
                    ctx.font = "12px serif";
                    if (i == 0){
                        ctx.fillText(min_time_zone, [steps_array[i] * unit] -unit/6, height_for_x_lapses + 0.02*height); //bro it works, omg --> you ask why, very simple, it took me like 15hours, to finish this shit
                    }
                    else{
                        ctx.fillText((min_time_zone + Math.round(steps_array[i] * 10)/10), steps_array[i] * unit -unit/6, height_for_x_lapses + 0.02*height); //extremely complicated, it depends on the definition of the general time zone
                    }
                }
                ctx.font = "20px serif";
                ctx.fillText("x in hours", x_whole_axe_length + x_axe_name_diff,1);
            }
            else{
                console.log("Something happened, data could not be read, please check your Sensorbox");
            }
        break;

        case "Today":
            lapse = 24; //1, because of 0 time
            unit = x_point_length / lapse;
            //clearRect(...,...,unit/2 + x_axe_length-1,...) --> +unit/2, because of the x-axe line shift, thats why, okay?
            ctx.clearRect(-unit/2, 1, unit/2 + x_axe_length-1, height_for_x_lapses + 0.02*height + 12); //clearing the axe, so it is cleared
                                                              //we are beginning at 1, because of line width
            measure_text_x_axe_and_delete(); //deleting the text

            for (let i = 0; i <= lapse; i++) {    
                ctx.beginPath();
                ctx.moveTo(i * unit, 0);
                ctx.lineTo(i*unit, height_for_x_lapses);
                ctx.lineWidth = 2; //this one
                ctx.strokeStyle = '#000000';
                ctx.stroke();
                //lapses naming
                ctx.font = "12px serif";

                if (i < 10){
                    ctx.fillText( "0" + [i] + ":00", [-unit/3 + i*unit], height_for_x_lapses + 0.02*height);
                }
                else{
                    ctx.fillText( [i] + ":00", [-unit/3 + i*unit], height_for_x_lapses + 0.02*height);
                }
            } 
            ctx.font = "20px serif";
            ctx.fillText("x in hours", x_whole_axe_length + x_axe_name_diff,1);  
        break;
        
        case "Yesterday":
            lapse = 24; //1, because of 0 time
            unit = x_point_length / lapse; //for each time lapse

            ctx.clearRect(-unit/2, 1, unit/2 + x_axe_length-1, height_for_x_lapses + 0.02*height + 12); //clearing the axe, so it is cleared --> 12, is the font size of the text
                                                              //we are beginning at 1, because of line width
            measure_text_x_axe_and_delete(); //deleting the text

            for (let i = 0; i <= lapse; i++) {    
                ctx.beginPath();
                ctx.moveTo(i * unit, 0);
                ctx.lineTo(i*unit, height_for_x_lapses);
                ctx.lineWidth = 2;
                ctx.strokeStyle = '#000000';
                ctx.stroke();
                //lapses naming
                ctx.font = "12px serif";

                if (i < 10){
                    ctx.fillText( "0" + [i] + ":00", [-unit/3 + i*unit], height_for_x_lapses + 0.02*height);
                }
                else{
                    ctx.fillText( [i] + ":00", [-unit/3 + i*unit], height_for_x_lapses + 0.02*height);
                }
            }
            ctx.font = "20px serif";
            ctx.fillText("x in hours", x_whole_axe_length + x_axe_name_diff,1);
        break;

        case "2 Days in a row":
            lapse = 24; //1, because of 0 time
            unit = x_point_length / lapse; //for each time lapse

            ctx.clearRect(-unit/2, 1, unit/2 + x_axe_length-1, height_for_x_lapses + 0.02*height + 12); //clearing the axe, so it is cleared
                                                              //we are beginning at 1, because of line width
            measure_text_x_axe_and_delete(); //deleting the text

            for (let i = 0; i <= lapse; i++) {    
                ctx.beginPath();
                ctx.moveTo(i * unit, 0);
                ctx.lineTo(i*unit, height_for_x_lapses);
                ctx.lineWidth = 2;
                ctx.strokeStyle = '#000000';
                ctx.stroke();
                //lapses naming
                ctx.font = "12px serif";

                if (i <= 12){
                    if (i <= 6){
                        if (i <= 4){
                            ctx.fillText( "0" + [i]*2 + ":00", [-unit/3 + i*unit], height_for_x_lapses + 0.02*height);
                        }
                        else{
                            ctx.fillText( [i]*2 + ":00", [-unit/3 + i*unit], height_for_x_lapses + 0.02*height);
                        }
                    }
                    else{
                        ctx.fillText( [i]*2 + ":00", [-unit/3 + i*unit], height_for_x_lapses + 0.02*height);
                    }
                }
                else if (i > 12){
                    if (i <= 18){
                        if (i <= 16){
                            ctx.fillText( "0" + (2 + ([i] - 13)*2) + ":00", [-unit/3 + i*unit], height_for_x_lapses + 0.02*height);
                        }
                        else{
                            ctx.fillText( (2 + ([i] - 13)*2) + ":00", [-unit/3 + i*unit], height_for_x_lapses + 0.02*height);
                        }
                    }
                    else{
                        ctx.fillText( (2 + ([i] - 13)*2) + ":00", [-unit/3 + i*unit], height_for_x_lapses + 0.02*height);
                    }
                }
            }
            ctx.font = "20px serif";
            ctx.fillText("x in hours", x_whole_axe_length + x_axe_name_diff,1);
        break;
    }
}

function measure_text_x_axe_and_delete(){ //for x-axe arrow naming
    const canvas = document.getElementById('graph');
    const ctx = canvas.getContext("2d"); //getting the features of the canvas, so i can promptly edit it AKA the context ... ctx
    
    //getting the width
    const width = canvas.width;

    const x_axe_name_diff = width * 0.005; //dif ... difference

    ctx.lineWidth = 0.5;
    ctx.strokeStyle = '#000000'; //if strokestyle was not set correctly --> changes: it will now be set in the occupied function AKA in this function
    
    //getting the measurments - the naming of the x-axe will be measured here
    var x_months_length = ctx.measureText("x in months").width;
    var x_days_length = ctx.measureText("x in days").width;
    var x_hours_length = ctx.measureText("x in hours").width;

    //now deleting
    //days
    ctx.clearRect(x_whole_axe_length + x_axe_name_diff, 1, x_days_length, -20); //20px, we have to do "ctx.font = 20px serif", because it's allignment is up .... (...,1,...,...) because of the line width
    ctx.clearRect(x_whole_axe_length + x_axe_name_diff, 1, x_days_length, x_axe_name_diff); //double insurance, sometimes it deletes almost everything, but not all the text --> x_axe_name_diff, because it's deletes only the neccessary lines
    //x_hours
    ctx.clearRect(x_whole_axe_length + x_axe_name_diff, 1, x_hours_length, -20);
    ctx.clearRect(x_whole_axe_length + x_axe_name_diff, 1, x_hours_length, x_axe_name_diff); //same here
    //x_months
    ctx.clearRect(x_whole_axe_length + x_axe_name_diff, 1, x_months_length, -20);
    ctx.clearRect(x_whole_axe_length + x_axe_name_diff, 1, x_months_length, x_axe_name_diff);
}

function measure_text_y_axe_and_delete(){ //for y-axe arrow naming
    const canvas = document.getElementById('graph');
    const ctx = canvas.getContext("2d"); //getting the features of the canvas, so i can promptly edit it AKA the context ... ctx
    
    //getting the width
    const width = canvas.width;

    const y_axe_name_diff = width * 0.005; //diff ... difference

    ctx.lineWidth = 0.5;
    ctx.strokeStyle = '#000000'; //if strokestyle was not set correctly --> changes: it will now be set in the occupied function AKA in this function
    
    //getting the measurments - the naming of the x-axe will be measured here
    var y_temp_length = ctx.measureText("Temperature").width;
    var y_airmois_length = ctx.measureText("Airmoisure").width;
    var y_airpress_length = ctx.measureText("Airpressure").width;
    var y_ozon_length = ctx.measureText("Ozon").width;

    //now deleting
    //temperature
    ctx.clearRect(-1, -[y_whole_axe_length + y_axe_name_diff], y_temp_length, -20); //20px, we have to do "ctx.font = 20px serif", because it's allignment is up .... (...,1,...,...) because of the line width
    ctx.clearRect(-1, -[y_whole_axe_length + y_axe_name_diff], y_temp_length, y_axe_name_diff); //double insurance, sometimes it deletes almost everything, but not all the text
    //airmoisure
    ctx.clearRect(-1, -[y_whole_axe_length + y_axe_name_diff], y_airmois_length, -20);
    ctx.clearRect(-1, -[y_whole_axe_length + y_axe_name_diff], y_airmois_length, y_axe_name_diff); //same here
    //airpressure
    ctx.clearRect(-1, -[y_whole_axe_length + y_axe_name_diff], y_airpress_length, -20);
    ctx.clearRect(-1, -[y_whole_axe_length + y_axe_name_diff], y_airpress_length, y_axe_name_diff);
    //ozon
    ctx.clearRect(-1, -[y_whole_axe_length + y_axe_name_diff], y_ozon_length, -20);
    ctx.clearRect(-1, -[y_whole_axe_length + y_axe_name_diff], y_ozon_length, y_axe_name_diff);
}

function sensor_setting(){
    //the same like setting_time() --> yeaaah, it was the same, but after i changed a few things its not anymore, the same function
    const sensor = document.getElementById("box_sens").value;
    const zone = document.getElementById("box_time").value;
    let lapse = 15;
    let unit = 0;  //for each time lapse

    //here i will draw a xy Graph with legends
    const canvas = document.getElementById('graph');
    const ctx = canvas.getContext("2d"); //getting the the features of the canvas, so i can promptly edit it AKA the context ... ctx
    
    //getting the height and width
    const width = canvas.width;
    const height = canvas.height;

    const y_axe_name_diff = width * 0.005; //diff ... difference

    switch (sensor) { //the scaling depends on the sensors
        case "Temperature":
            switch (zone) {
                case "Current-Data-Time": //it will only activate, if we use "Current-Data-Time"
                    if (read_data_bool == true && save_data_bool == true && data_division_temp != 0){
                        lapse = data_max_temp - data_min_temp;
                        unit = data_division_temp;

                        ctx.clearRect(-1, 2, -width*0.1, -y_axe_length-1); // clearrect(..,2,..,...), because if do less than that, we well we will delete the arrow.
                                                                           // Why, you ask, very simple cause this a self drawn graph not like any other you find online
                        measure_text_y_axe_and_delete(); //deleting the text aka arrow names, so conflicts wont happen

                        const steps = lapse/10;
                        let steps_array = [];

                        for (let i = 0; i <= 10; i++) {
                            steps_array[i] = steps * i;                            
                        }

                        for (let i = 0; i < steps_array.length; i++) {
                            //line generating
                            ctx.beginPath();
                            ctx.moveTo(0, -steps_array[i] * unit);
                            ctx.lineTo(-width*0.01, -steps_array[i] * unit); // -i*unit --> because we have a translate point, meaning everything under the translate point is positive and everything above it, is negative
                            ctx.lineWidth = 2;
                            ctx.strokeStyle = '#000000';
                            ctx.stroke();
                            //giving numbers (names) to the lines with corosponding --> -75°C...250°C --> our complete range is 325°C --> meanig per lapse we have --> 32.5 degree difference
                            ctx.font = "20px serif";

                            ctx.fillText([data_min_temp + Number((Math.round(steps_array[i] * 10)/10).toPrecision(2))] + "°C", -width*0.06, -steps_array[i] * unit);
                        }

                        ctx.font = "20px serif";
                        ctx.fillText("Temperature", -1, -[y_whole_axe_length + y_axe_name_diff]); //arrow naming
                    }
                break;
            
                default:
                    ctx.clearRect(-1, 2, -width*0.1, -y_axe_length-1); // clearrect(..,2,..,...), because if do less than that, we well we will delete the arrow.
                                                                       // Why, you ask, very simple cause this a self drawn graph not like any other you find online
                    measure_text_y_axe_and_delete(); //deleting the text aka arrow names, so conflicts wont happen
                    //-75°C...250°C
                    lapse = 10;
                    unit = y_point_length / lapse; //for each time lapse
                    for (let i = 0; i <= lapse; i++) {    
                        //line generating
                        ctx.beginPath();
                        ctx.moveTo(0, -i * unit);
                        ctx.lineTo(-width*0.01, -i*unit); // -i*unit --> because we have a translate point, meaning everything under the translate point is positive and everything above it, is negative
                        ctx.lineWidth = 2;
                        ctx.strokeStyle = '#000000';
                        ctx.stroke();
                        //giving numbers (names) to the lines with corosponding --> -75°C...250°C --> our complete range are 325°C --> meanig per lapse we have --> 32.5 degree difference
                        ctx.font = "20px serif";
                        const unit_temp = 325/lapse;
                        ctx.fillText([-75+(i*unit_temp)] + "°C", -width*0.06, -i*unit);
                    }
                    ctx.font = "20px serif";
                    ctx.fillText("Temperature", -1, -[y_whole_axe_length + y_axe_name_diff]); //arrow naming
                break;
            }
            
        break;

        case "Airmoisure":
            switch (zone) {
                case "Current-Data-Time": //it will only activate, if we use "Current-Data-Time"
                    if (read_data_bool == true && save_data_bool == true && data_division_hpp != 0){
                        lapse = data_max_hpp - data_min_hpp;
                        unit = data_division_hpp;

                        ctx.clearRect(-1, 2, -width*0.1, -y_axe_length-1); // clearrect(..,2,..,...), because if do less than that, we well we will delete the arrow.
                                                                           // Why, you ask, very simple cause this a self drawn graph not like any other you find online
                        measure_text_y_axe_and_delete(); //deleting the text aka arrow names, so conflicts wont happen

                        const steps = lapse/10;
                        let steps_array = [];

                        for (let i = 0; i <= 10; i++) {
                            steps_array[i] = steps * i;                            
                        }

                        for (let i = 0; i < steps_array.length; i++) {
                            //line generating
                            ctx.beginPath();
                            ctx.moveTo(0, -steps_array[i] * unit);
                            ctx.lineTo(-width*0.01, -steps_array[i] * unit); // -i*unit --> because we have a translate point, meaning everything under the translate point is positive and everything above it, is negative
                            ctx.lineWidth = 2;
                            ctx.strokeStyle = '#000000';
                            ctx.stroke();
                            //giving numbers (names) to the lines with corosponding --> 0 ... 100% --> our complete range is 100%
                            ctx.font = "20px serif";

                            ctx.fillText([data_min_hpp + Number((Math.round(steps_array[i] * 10)/10).toPrecision(2))] + "%", -width*0.06, -steps_array[i] * unit);
                        }

                        ctx.font = "20px serif";
                        ctx.fillText("Temperature", -1, -[y_whole_axe_length + y_axe_name_diff]); //arrow naming
                    }
                break;
            
                default:
                    ctx.clearRect(-1, 2, -width*0.1, -y_axe_length-1); //clearing the axe, so it is cleared
                    measure_text_y_axe_and_delete(); //deleting the text aka arrow names, so conflicts wont happen
                    //0...100% RH
                    lapse = 10; 
                    unit = y_point_length / lapse; //for each time lapse
                    for (let i = 0; i <= lapse; i++) {    
                        //line generating
                        ctx.beginPath();
                        ctx.moveTo(0, -i * unit);
                        ctx.lineTo(-width*0.01, -i*unit); // -i*unit --> because we have a translate point, meaning everything under the translate point is positive and everything above it, is negative
                        ctx.lineWidth = 2;
                        ctx.strokeStyle = '#000000';
                        ctx.stroke();
                        //giving numbers (names) to the lines with corosponding --> 0%...100% --> our complete range is 100% --> meanig per lapse we have --> 1% degree difference
                        ctx.font = "20px serif";
                        const unit_temp = 100/lapse;
                        ctx.fillText([(i*unit_temp)] + "%", -width*0.06, -i*unit);
                    }
                    ctx.font = "20px serif";
                    ctx.fillText("Airmoisure", -1, -[y_whole_axe_length + y_axe_name_diff]); //arrow naming
                break;
            }
        break;

        case "Airpressure":
            switch (zone) {
                case "Current-Data-Time": //it will only activate, if we use "Current-Data-Time"
                    if (read_data_bool == true && save_data_bool == true && data_division_bme != 0){
                        lapse = data_max_bme - data_min_bme;
                        unit = data_division_bme;

                        ctx.clearRect(-1, 2, -width*0.1, -y_axe_length-1); // clearrect(..,2,..,...), because if do less than that, we well we will delete the arrow.
                                                                           // Why, you ask, very simple cause this a self drawn graph not like any other you find online
                        measure_text_y_axe_and_delete(); //deleting the text aka arrow names, so conflicts wont happen

                        const steps = lapse/10;
                        let steps_array = [];

                        for (let i = 0; i <= 10; i++) {
                            steps_array[i] = steps * i;                            
                        }

                        for (let i = 0; i < steps_array.length; i++) {
                            //line generating
                            ctx.beginPath();
                            ctx.moveTo(0, -steps_array[i] * unit);
                            ctx.lineTo(-width*0.01, -steps_array[i] * unit); // -i*unit --> because we have a translate point, meaning everything under the translate point is positive and everything above it, is negative
                            ctx.lineWidth = 2;
                            ctx.strokeStyle = '#000000';
                            ctx.stroke();
                            //giving numbers (names) to the lines with corosponding --> 300hPa ... 1100hPa --> our max difference is 800hPa
                            ctx.font = "20px serif";

                            ctx.fillText([data_min_bme + (Math.round(Number(steps_array[i])))] + "hPa", -width*0.06, -steps_array[i] * unit);
                        }

                        ctx.font = "20px serif";
                        ctx.fillText("Airpressure", -1, -[y_whole_axe_length + y_axe_name_diff]); //arrow naming
                    }
                break;

                default:
                    ctx.clearRect(-1, 2, -width*0.1, -y_axe_length-1); //clearing the axe, so it is cleared
                    measure_text_y_axe_and_delete(); //deleting the text aka arrow names, so conflicts wont happen
                    //300hPa...1100hPa
                    lapse = 10; 
                    unit = y_point_length / lapse; //for each time lapse
                    for (let i = 0; i <= lapse; i++) {    
                        //line generating
                        ctx.beginPath();
                        ctx.moveTo(0, -i * unit);
                        ctx.lineTo(-width*0.01, -i*unit); // -i*unit --> because we have a translate point, meaning everything under the translate point is positive and everything above it, is negative
                        ctx.lineWidth = 2;
                        ctx.strokeStyle = '#000000';
                        ctx.stroke();
                        //giving numbers (names) to the lines with corosponding 
                        ctx.font = "20px serif";
                        const unit_temp = 800/lapse; //1100hPa - 300hPa = 800hPa
                        ctx.fillText([300 + (i*unit_temp)] + "hPa", -width*0.06, -i*unit);
                    }
                    ctx.font = "20px serif";
                    ctx.fillText("Airpressure", -1, -[y_whole_axe_length + y_axe_name_diff]); //arrow naming
                break;
            }    
        break;

        case "Gas":
            switch (zone) {
                case "Current-Data-Time": //it will only activate, if we user Current-Data-Time
                    if (read_data_bool == true && save_data_bool == true && data_division_ozon != 0){
                        lapse = data_max_ozon - data_min_ozon;
                        unit = data_division_ozon;

                        ctx.clearRect(-1, 2, -width*0.1, -y_axe_length-1); // clearrect(..,2,..,...), because if do less than that, we well we will delete the arrow. Why, you ask, very simple cause this a self drawn graph not like any other you find online
                        measure_text_y_axe_and_delete(); //deleting the text aka arrow names, so conflicts wont happen

                        const steps = lapse/10;
                        let steps_array = [];

                        for (let i = 0; i <= 10; i++) {
                            steps_array[i] = steps * i;                            
                        }

                        for (let i = 0; i < steps_array.length; i++) {
                            //line generating
                            ctx.beginPath();
                            ctx.moveTo(0, -steps_array[i] * unit);
                            ctx.lineTo(-width*0.01, -steps_array[i] * unit); // -i*unit --> because we have a translate point, meaning everything under the translate point is positive and everything above it, is negative
                            ctx.lineWidth = 2;
                            ctx.strokeStyle = '#000000';
                            ctx.stroke();
                            //giving numbers (names) to the lines with corosponding --> 0 ... 100% --> our complete range are 100%
                            ctx.font = "20px serif";

                            ctx.fillText([data_min_ozon + Number((Math.round(steps_array[i] * 10)/10).toPrecision(2))] + "ppm", -width*0.06, -steps_array[i] * unit);
                        }

                        ctx.font = "20px serif";
                        ctx.fillText("Ozon", -1, -[y_whole_axe_length + y_axe_name_diff]); //arrow naming
                    }
                break;

                default:
                    //i dont know we have to test it first --> now i know how to do it
                    ctx.clearRect(-1, 2, -width*0.1, -y_axe_length-1); //clearing the axe, so it is cleared
                    measure_text_y_axe_and_delete(); //deleting the text aka arrow names, so conflicts wont happen
                    //0% ... 100%
                    lapse = 10; 
                    unit = y_point_length / lapse; //for each time lapse
                    for (let i = 0; i <= lapse; i++) {    
                        //line generating
                        ctx.beginPath();
                        ctx.moveTo(0, -i * unit);
                        ctx.lineTo(-width*0.01, -i*unit); // -i*unit --> because we have a translate point, meaning everything under the translate point is positive and everything above it, is negative
                        ctx.lineWidth = 2;
                        ctx.strokeStyle = '#000000';
                        ctx.stroke();
                        //giving numbers (names) to the lines with corosponding 
                        ctx.font = "20px serif";
                        const unit_temp = 100/lapse;
                        ctx.fillText((i*unit_temp) + "ppm", -width*0.06, -i*unit);
                    }
                    ctx.font = "20px serif";
                    ctx.fillText("Ozon", -1, -[y_whole_axe_length + y_axe_name_diff]); //arrow naming
                break;
            }
        break;
    }
}

function HTTP_READ(){ //here i will set and save the values for the sensor data
    //bme should also measure temperature 
    //ozon in ppm, does it change
    //first i will get the generated data, which is --> og's comments from me

    read_data_bool = false; //for the time-zone --> time-now

    //reading the whole data's
    Read_from_Ozon(); //test-method for xaamp webserver
    Read_from_bme(); //test-method for xaamp webserver
    Read_from_hpp(); //test-method for xaamp webserver
    Read_from_temp(); //test-method for xaamp webserver

    read_data_bool = true;
}

function HTTP_SAVE(){ //for saving the whole data, so i can extract the coordinates (canvas)
    //saving the data
    save_data_bool = false; //for the time zone
    
    //------------------------------------------------//
    const result_values_ozon = save_drawing_data_for_ozon();
    if (result_values_ozon[2] != 0){ //WHYYYY, you ask? Cause you asked for it. I am obviously joking, with this if statement, i am protected against invalid results from my function
        data_max_ozon = result_values_ozon[0];
        data_min_ozon = result_values_ozon[1];
        data_division_ozon = result_values_ozon[2];
    }

    const result_values_bme = save_drawing_data_for_bme();
    if (result_values_bme[2] != 0){
        data_max_bme = result_values_bme[0];
        data_min_bme = result_values_bme[1];
        data_division_bme = result_values_bme[2];
    }

    const result_values_hpp = save_drawing_data_for_hpp();
    if (result_values_hpp[2] != 0){
        data_max_hpp = result_values_hpp[0];
        data_min_hpp = result_values_hpp[1];
        data_division_hpp = result_values_hpp[2];
    }

    const result_values_temp = save_drawing_data_for_temp();
    if (result_values_temp[2] != 0){
        data_max_temp = result_values_temp[0];
        data_min_temp = result_values_temp[1];
        data_division_temp = result_values_temp[2];
    }
    //------------------------------------------------//
    save_data_bool = true;
}

//this method can be used for test datas
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------//
function Read_from_hpp(){ 
    const sensor_name = '"msgHPP":'; //for splitting the name of the sensor from the sensor-data
    fetchUserInfo("localhost", "Diplomarbeit", "Website", "test_hpp.http", data_hpp, sensor_name, data_hpp_no_sym, hpp_split, hpp_time, hpp_data, ":HPP:"); //test data
}

function Read_from_Ozon(){
    const sensor_name = '"msgOZON":'; //for splitting the name of the sensor from the sensor-data
    fetchUserInfo("localhost", "Diplomarbeit", "Website", "test_ozon.http", data_ozon, sensor_name, data_ozon_no_sym, ozon_split, ozon_time, ozon_data, ":OZON:"); //test data
}

function Read_from_temp(){
    const sensor_name = '"msgTEMP":'; //for splitting the name of the sensor from the sensor-data
    fetchUserInfo("localhost", "Diplomarbeit", "Website", "test_temp.http", data_temp, sensor_name, data_temp_no_sym, temp_split, temp_time, temp_data, ":PT:"); //test data
}

function Read_from_bme(){ //this method will later be implemented in HTTP_SET
    const sensor_name = '"msgBME":'; //for splitting the name of the sensor from the sensor-data
    fetchUserInfo("localhost", "Diplomarbeit", "Website", "test_bme.http", data_bme, sensor_name, data_bme_no_sym, bme_split, bme_time, bme_data, ":BME:"); //test data
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------//


function Read_all(){
    //here we will read: all Sensor-data --> e.g. BME, HPP, etc.
    fetch_ALL_INFO("sensorbox.com", "sd", "/LIVE", sensor_mac_addresses_sensorbox, sensor_all_data_sensorbox);
}

function save_drawing_data_for_ozon(){
    //reading zone ... getting the element and reading the value, which is currently used e.g. sellected
    const zone = document.getElementById("box_time").value;

    //Range --> [0-100]ppm
    if (ozon_data.length >= 1){

        ozon_canvas_x = [];
        ozon_canvas_y = [];

        let y_coordinates = []; //saving the coords, for drawing, or else it will be way too complicated

        let ozon_max = 0; //max 100%
        let ozon_min = 0; //min 0%
        let ozon_division = 0; //division factor

        switch (zone) {
            case "Current-Data-Time":
                ozon_max = Number(ozon_data[0]); //max 100%
                ozon_min = Number(ozon_data[0]); //min 0%

                for (let i = 0; i < ozon_data.length; i++) {
                    if (ozon_max <= Number(ozon_data[i])){ //we have to add "Number()"", because its a string, if we dont do that, yeeeah, our values will be completly wrong.
                        ozon_max = Number(ozon_data[i]); //biggest value
                    }
                    if (ozon_min >= Number(ozon_data[i])){
                        ozon_min = Number(ozon_data[i]); //lowest value
                    }
                }

                ozon_division = y_point_length / (ozon_max - ozon_min); //division factor

                for (let i = 0; i < ozon_data.length; i++) { //y-coordinates
                    let dy_ozon = ozon_max - Number(ozon_data[i]); //max difference 100%

                    switch (dy_ozon) { //same function like before, but more general
                        case 0: //max value of 100% reached
                            y_coordinates[i] = y_point_length;
                            ozon_canvas_y[i] = y_coordinates[i];
                        break;

                        case (ozon_max - ozon_min): //lowest value possible is 0%
                            y_coordinates[i] = 0;
                            ozon_canvas_y[i] = y_coordinates[i];
                        break;

                        default:
                            y_coordinates[i] = ozon_division * (Number(ozon_data[i]) - ozon_min);
                            ozon_canvas_y[i] = y_coordinates[i];
                        break;
                    }

                //////////////////check///////////////////////
                //////////////////////////////////////////////       
                console.log(i + " . " + y_coordinates[i] + " - Y-Values");
                //////////////////////////////////////////////
                }   
            break;
        
            default:
                //the max difference for % value is 100 --> 100% - 0%, very important
                const ozon_100 = y_point_length; //100% measurement
                const ozon_10 = ozon_100 / 10; //10% measurement
                const ozon_1 = ozon_10 / 10; //1% measurement
                
                for (let i = 0; i < ozon_data.length; i++) { //y-coordinates
                    let dy_ozon = ozon_max_value - Number(ozon_data[i]);

                    switch (dy_ozon) { // 0 and 100 are special cases, extremeeeely unlikely to happen --> SSR Rank, be carefull
                        case 0: //max value of 100% reached
                            y_coordinates[i] = ozon_100;
                            ozon_canvas_y[i] = y_coordinates[i];
                        break;

                        case 100: //lowest value of 0% reached
                            y_coordinates[i] = 0;
                            ozon_canvas_y[i] = y_coordinates[i];
                        break;

                        default:
                            y_coordinates[i] = ozon_1 * Number(ozon_data[i]);
                            ozon_canvas_y[i] = y_coordinates[i];        
                        break;
                    }

                    //////////////////check///////////////////////
                    //////////////////////////////////////////////       
                    console.log(i + " . " + y_coordinates[i] + " - Y-Values");
                    //////////////////////////////////////////////
                }
            break;
        }

        switch (zone) { //selecting the right time zone, hehe, i know this method, has 1000 aura, because of its unique properties
            case "Current-Data-Time":
                let result_values = sensor_time_calculate_actual_coordinates_universal(ozon_canvas_x, ozon_time);
                time_max_ozon = result_values[0];
                time_min_ozon = result_values[1];
                time_division_ozon = result_values[2];
            break;

            case "Today":
                sensor_time_calculate_their_Coordinates_24h(ozon_canvas_x, ozon_time);
            break;

            case "Yesterday":
                sensor_time_calculate_their_Coordinates_24h(ozon_canvas_x, ozon_time);
            break;

            case "2 Days in a row":

            break;
        } 

        if (ozon_division != 0){
            return [ozon_max, ozon_min, ozon_division];
        }
        else{
            return [0, 0, 0];
        }
    }
    else{
        console.log("Could not retrieve the corrosponding OZON-sensor data");
    }
}

function save_drawing_data_for_temp(){
    //reading zone ... getting the element and reading the value, which is currently used e.g. selected
    const zone = document.getElementById("box_time").value;

    //Range --> -75°C...250°C
    if (temp_data.length >= 1){

        temp_canvas_x = [];
        temp_canvas_y = [];

        let y_coordinates = []; //saving the coords, for drawing, or else it will be way too complicated

        let temp_max = 0; //max 250°C
        let temp_min = 0; //min -75°C
        let temp_division = 0; //division factor

        switch (zone) {
            case "Current-Data-Time":
                temp_max = Number(temp_data[0]); //max 250°C
                temp_min = Number(temp_data[0]); //min -75°C

                for (let i = 0; i < temp_data.length; i++) {
                    if (temp_max <= Number(temp_data[i])){ //we have to add Number, because its a string, if we dont do that, yeeeah, our values will be completly wrong.
                        temp_max = Number(temp_data[i]); //biggest value
                    }
                    if (temp_min >= Number(temp_data[i])){
                        temp_min = Number(temp_data[i]); //lowest value
                    }
                }
                temp_division = y_point_length / (temp_max - temp_min); //division factor

                for (let i = 0; i < temp_data.length; i++) { //y-coordinates
                    let dy_temp = temp_max - Number(temp_data[i]); //max difference 325°C

                    switch (dy_temp) { //same function like before, but more general
                        case 0: //max value of 250°C reached
                            y_coordinates[i] = y_point_length;
                            temp_canvas_y[i] = y_coordinates[i];
                        break;

                        case (temp_max - temp_min): //lowest value possible is -75°C 
                            y_coordinates[i] = 0;
                            temp_canvas_y[i] = y_coordinates[i];
                        break;

                        default:
                            if (temp_min >= 0){ //min area is greater than 0°C
                                y_coordinates[i] = temp_division * (Number(temp_data[i]) - temp_min);
                                temp_canvas_y[i] = y_coordinates[i];
                            }
                            else if (temp_min < 0 && temp_max < 0){ //the whole measurement range is less than 0 
                                y_coordinates[i] = temp_division * (Math.abs(temp_min) + Number(temp_data[i]));
                                temp_canvas_y[i] = y_coordinates[i];    
                            }
                            else if (temp_min < 0 && temp_max > 0){ //negative and positiv range area
                                if (Number(temp_data[i]) < 0){
                                    y_coordinates[i] = temp_division * (Math.abs(temp_min) + Number(temp_data[i]));
                                    temp_canvas_y[i] = y_coordinates[i];
                                }   
                                else{
                                    y_coordinates[i] = temp_division * (Number(temp_data[i]) + Math.abs(temp_min)); //begin where ever you like
                                    temp_canvas_y[i] = y_coordinates[i];
                                }   
                            }
                        break;
                    }

                //////////////////check///////////////////////
                //////////////////////////////////////////////       
                console.log(i + " . " + y_coordinates[i] + " - Y-Values");
                //////////////////////////////////////////////
                }          
            break;
        
            default:
                //the max difference for % value is 100 --> 100% - 0%, very important
                const temp_325 = y_point_length; //325°C measurement
                const temp_32_5 = temp_325 / 10; //32.5°C measurement
                const temp_3_25 = temp_32_5 / 10; //3.25°C measurement
                const temp_1 = temp_3_25 / 3.25; //1°C measurement
                
                for (let i = 0; i < temp_data.length; i++) { //y-coordinates
                    let dy_temp = temp_max_value - temp_data[i];

                    switch (dy_temp) { // 0 and 100 are special cases, extremeeeely unlikely to happen --> SSR Rank, be carefull
                        case 0: //max value of 250°C reached
                            y_coordinates[i] = temp_325;
                            temp_canvas_y[i] = y_coordinates[i];
                        break;

                        case 325: //lowest value of -75°C reached
                            y_coordinates[i] = 0;
                            temp_canvas_y[i] = y_coordinates[i];
                        break;

                        default:
                            if (Number(temp_data[i]) < 0){
                                y_coordinates[i] = temp_1 * (75 + Number(temp_data[i])); //beginning at -75°C
                                temp_canvas_y[i] = y_coordinates[i];
                            }
                            else{
                                y_coordinates[i] = temp_1 * (Number(temp_data[i]) + 75); //beginning at 0°C
                                temp_canvas_y[i] = y_coordinates[i];
                            }
                        break;
                    }

                //////////////////check///////////////////////
                //////////////////////////////////////////////       
                console.log(i + " . " + y_coordinates[i] + " - Y-Values");
                //////////////////////////////////////////////
                }
            break;
        }     

        switch (zone) {
            case "Current-Data-Time":
                let result_values = sensor_time_calculate_actual_coordinates_universal(temp_canvas_x, temp_time);
                time_max_temp = result_values[0];
                time_min_temp = result_values[1];
                time_division_temp = result_values[2];
            break;

            case "Today":
                sensor_time_calculate_their_Coordinates_24h(temp_canvas_x, temp_time);
            break;

            case "Yesterday":
                sensor_time_calculate_their_Coordinates_24h(temp_canvas_x, temp_time);
            break;

            case "2 Days in a row":

            break;
        } 

        if (temp_division != 0){
            return [temp_max, temp_min, temp_division];
        }
        else{
            return [0, 0, 0];
        }
    }
    else{
        console.log("Could not retrieve the corrosponding Temp-sensor data");
    }
}

function save_drawing_data_for_hpp(){
    //reading zone ... getting the element and reading the value, which is currently used e.g. sellected
    const zone = document.getElementById("box_time").value;

    //Range --> [0-100]%
    if (hpp_data.length >= 1){ 

        hpp_canvas_x = [];
        hpp_canvas_y = [];

        let y_coordinates = []; //saving the coords, for drawing, or else it will be way too complicated

        let hpp_max = 0; //max 100%
        let hpp_min = 0; //min 0%
        let hpp_division = 0; //division factor

        switch (zone) {
            case "Current-Data-Time":
                hpp_max = Number(hpp_data[0]); //max 100%
                hpp_min = Number(hpp_data[0]); //min 0%

                for (let i = 0; i < hpp_data.length; i++) {
                    if (hpp_max <= Number(hpp_data[i])){ //we have to add Number, because its a string, if we dont do that, yeeeah, our values will be completly wrong.
                        hpp_max = Number(hpp_data[i]); //biggest value
                    }
                    if (hpp_min >= Number(hpp_data[i])){
                        hpp_min = Number(hpp_data[i]); //lowest value
                    }
                }

                hpp_division = y_point_length / (hpp_max - hpp_min); //division factor

                for (let i = 0; i < hpp_data.length; i++) { //y-coordinates
                    let dy_hpp = hpp_max - Number(hpp_data[i]); //max difference 100%

                    switch (dy_hpp) { //same function like before, but more general
                        case 0: //max value of 100% reached
                            y_coordinates[i] = y_point_length;
                            hpp_canvas_y[i] = y_coordinates[i];
                        break;

                        case (hpp_max - hpp_min): //lowest value possible is 0%
                            y_coordinates[i] = 0;
                            hpp_canvas_y[i] = y_coordinates[i];
                        break;

                        default:
                            y_coordinates[i] = hpp_division * (Number(hpp_data[i]) - hpp_min);
                            hpp_canvas_y[i] = y_coordinates[i];
                        break;
                    }

                //////////////////check///////////////////////
                //////////////////////////////////////////////       
                console.log(i + " . " + y_coordinates[i] + " - Y-Values");
                //////////////////////////////////////////////
                }   
            break;
        
            default:
                //the max difference for % value is 100 --> 100% - 0%, very important
                const hpp_100 = y_point_length; //100%
                const hpp_10 = hpp_100 / 10; //10%
                const hpp_1 = hpp_10 / 10; //1%
        
                for (let i = 0; i < hpp_data.length; i++) { //y-coordinates
                    let dy_hpp = hpp_max_value - Number(hpp_data[i]);

                    switch (dy_hpp) { // 0 and 100 are special cases, extremeeeely unlikely to happen --> SSR Rank, be carefull
                        case 0: //max value of 100% reached
                            y_coordinates[i] = hpp_100;
                            hpp_canvas_y[i] = y_coordinates[i];
                        break;

                        case 100: //lowest value of 0% reached
                            y_coordinates[i] = 0;
                            hpp_canvas_y[i] = y_coordinates[i];
                        break;

                        default:
                            y_coordinates[i] = hpp_1 * Number(hpp_data[i]); 
                            hpp_canvas_y[i] = y_coordinates[i];
                        break;
                    }

                    //////////////////check///////////////////////
                    //////////////////////////////////////////////       
                    console.log(i + " . " + y_coordinates[i] + " - Y-Values");
                    //////////////////////////////////////////////
                }
            break;
        }

        switch (zone) {
            case "Current-Data-Time":
                let result_values = sensor_time_calculate_actual_coordinates_universal(hpp_canvas_x, hpp_time);
                time_max_hpp = result_values[0];
                time_min_hpp = result_values[1];
                time_division_hpp = result_values[2];
            break;

            case "Today":
                sensor_time_calculate_their_Coordinates_24h(hpp_canvas_x, hpp_time);
            break;

            case "Yesterday":
                sensor_time_calculate_their_Coordinates_24h(hpp_canvas_x, hpp_time);
            break;

            case "2 Days in a row":

            break;
        }

        if (hpp_division != 0){
            return [hpp_max, hpp_min, hpp_division];
        }
        else{
            return [0, 0, 0];
        }
         
    }
    else{
        console.log("Could not retrieve the corrosponding HPP-sensor data");
    }
}

function save_drawing_data_for_bme(){
    //reading zone ... getting the element and reading the value, which is currently used e.g. sellected
    const zone = document.getElementById("box_time").value;

    if (bme_data.length >= 1){
        //reseting those arrays
        bme_canvas_x = [];
        bme_canvas_y = [];

        let y_coordinates = []; //saving the coords, for drawing, or else it will be way too complicated
    
        let bme_max = 0; //max 100%
        let bme_min = 0; //min 0%
        let bme_division = 0; //division factor

        switch (zone) {
            case "Current-Data-Time":
                bme_max = Number(bme_data[0]); //max 1100hPa
                bme_min = Number(bme_data[0]); //min 300hPa

                for (let i = 0; i < bme_data.length; i++) {
                    if (bme_max <= Number(bme_data[i])){ //we have to add Number, because its a string, if we dont do that, yeeeah, our values will be completly wrong.
                        bme_max = Number(bme_data[i]); //biggest value
                    }
                    if (bme_min >= Number(bme_data[i])){
                        bme_min = Number(bme_data[i]); //lowest value
                    }
                }

                bme_division = y_point_length / (bme_max - bme_min); //division factor

                for (let i = 0; i < bme_data.length; i++) { //y-coordinates
                    let dy_bme = bme_max - Number(bme_data[i]); //max difference 800hPa

                    switch (dy_bme) { //same function like before, but more general
                        case 0: //max value of 1100hPa reached
                            y_coordinates[i] = y_point_length;
                            bme_canvas_y[i] = y_coordinates[i];
                        break;

                        case (bme_max - bme_min): //lowest value possible reached of 300hPa
                            y_coordinates[i] = 0;
                            bme_canvas_y[i] = y_coordinates[i];
                        break;

                        default:
                            y_coordinates[i] = bme_division * (Number(bme_data[i]) - bme_min);
                            bme_canvas_y[i] = y_coordinates[i];
                        break;
                    }

                    //////////////////check///////////////////////
                    //////////////////////////////////////////////       
                    console.log(i + " . " + y_coordinates[i] + " - Y-Values");
                    //////////////////////////////////////////////
                }   
            break;

            default:
                //the max difference for hPa value is 800hPa --> 1100hPa - 300hPa, very important
                const hPa_800 = y_point_length; //800hPa measurement
                const hPa_80 = hPa_800 / 10; //80hPa measurement
                const hPa_8 = hPa_80 / 10; //8hPa measurement
                const hPa_1 = hPa_8 / 8; //1hPa measurement --> with this method i can clearly draw it, without it's extremely difficult to draw stuff like that
                
                for (let i = 0; i < bme_data.length; i++) { //y-coordinates
                    let dy_bme = bme_max_value - Number(bme_data[i]);

                    switch (dy_bme) { // 0 and 800 are special cases, extremeeeely unlikely to happen --> SSR Rank, be carefull
                        case 0: //max value of 1100hPa reached
                            y_coordinates[i] = hPa_800;
                            bme_canvas_y[i] = y_coordinates[i];
                        break;

                        case 800: //lowest value of 300hPa reached
                            y_coordinates[i] = 0;
                            bme_canvas_y[i] = y_coordinates[i];
                        break;

                        default:
                            if (bme_data[i] > 300){
                                y_coordinates[i] = hPa_1 * (Number(bme_data[i]) - 300); // -300, because we beging to drawing at 300hPa
                                bme_canvas_y[i] = y_coordinates[i];
                            }
                        break;
                    }

                    //////////////////check///////////////////////
                    //////////////////////////////////////////////       
                    console.log(i + " . " + y_coordinates[i] + " - Y-Values");
                    //////////////////////////////////////////////
                }
            break;

        }
        
        switch (zone) {
            case "Current-Data-Time":
                let result_values = sensor_time_calculate_actual_coordinates_universal(bme_canvas_x, bme_time);
                time_max_bme = result_values[0];
                time_min_bme = result_values[1];
                time_division_bme = result_values[2];
            break;

            case "Today":
                sensor_time_calculate_their_Coordinates_24h(bme_canvas_x, bme_time);
            break;

            case "Yesterday":
                sensor_time_calculate_their_Coordinates_24h(bme_canvas_x, bme_time);
            break;

            case "2 Days in a row":

            break;
        } 
        //bme_canvas_y = y_coordinates; //please dont ask me, why this line of code is not working --> changes: it has probably sth to do how arrays function in js. Obviously completely different compared to C#

        if (bme_division != 0){
            return [bme_max, bme_min, bme_division];
        }
        else{
            return [0, 0, 0];
        }
    }
    else{
        console.log("Could not retrieve the corrosponding BME-sensor data");
    }
}

function sensor_time_calculate_actual_coordinates_universal(x_coord, sensor_time){
    let x_coord_tempory = [];
    let saved_hours = []; //saving the calculated hours 

    let time_divided = 0; //same here

    //main process
    ////////////////////////
    for (let i = 0; i < sensor_time.length; i++) { //read sensor_time and save it
        let current_time = sensor_time[i].split(":"); //01:10:00 - [h]:[min]:[s]
        let hour_string = current_time[0]; //hour
        let min_string = current_time[1]; //minutes
        let sec_string = current_time[2]; //seconds

        if (Number(hour_string) >= 10){ //thats good
            hour_the_chosen = hour_string;
        }
        if (Number(min_string) >= 10){
            min_the_chosen = min_string;
        }
        if (Number(sec_string) >= 10){
            sec_the_chosen = sec_string;
        }
        if (Number(hour_string) < 10){
            hour_the_chosen = hour_string.split("0")[1]; //like 01 -->string[1] ... 1
        }
        if (Number(min_string) < 10){
            min_the_chosen = min_string.split("0")[1];
        }
        if (Number(sec_string) < 10){
            sec_the_chosen = sec_string.split("0")[1];
        }

        let calculate_per_1h_from_this = Number(hour_the_chosen) + Number(min_the_chosen/60) + Number(sec_the_chosen/360); //like 12.5h
        saved_hours[i] = calculate_per_1h_from_this; //adding the current hours to my list, so that i can promptly work with them
    }

    let temp_max = saved_hours[0]; //setting values, so that i can start comparing
    let temp_min = saved_hours[0]; //same here

    //here i will select the biggest and the lowest time --> for the graph and for the calculations
    for (let i = 0; i < saved_hours.length; i++) { //for 2x method ---> yk what i am talking about. With that i can clearly select the biggest data and the smallest data. chad for-method.
        if (temp_max <= saved_hours[i]){
            temp_max = saved_hours[i]; //setting temporary biggest value
        }
        if (temp_min >= saved_hours[i]){ 
            temp_min = saved_hours[i]; //same here, but the smallest value
        }
    }

    for (let i = 0; i < sensor_time.length; i++) { //read sensor_time and save it
        //the max time value for the sensors -- i concluded, that a day-time zone will be programmed first and the other participants like 2 weeks shown line, etc. will be connected to this 24-hour data --> still same thought, but 2 weeks wont appear anymore :(
        let time_all = x_point_length; //general time - coordinate
        time_divided = time_all / (temp_max - temp_min); //divided time  - coordinate


        //check values
        //////////////////////////////////////////////
        console.log(`"Sensor max time: ${temp_max} | Sensor min time: ${temp_min} | Sensor time division: ${time_divided} --- ${sensor_time[1]}"`);
        //////////////////////////////////////////////

        let dx_ultimate_time = temp_max - saved_hours[i];

        switch (dx_ultimate_time) { // 0 and 24 are not always there
            case 0:
                x_coord_tempory[i] = time_all;
            break;
        
            case (temp_max - temp_min):
                x_coord_tempory[i] = time_divided * temp_min;
            break;

            default:
                x_coord_tempory[i] = time_divided * (saved_hours[i] - temp_min);
            break;
        }
        //////////////////check////////////////
        ///////////////////////////////////////
        console.log(i + " . " + x_coord_tempory[i] + " - X-Values");
        ///////////////////////////////////////
    }

    //now we will shift the coordinates
    for (let i = 0; i < x_coord_tempory.length; i++) {
        x_coord_tempory[i] -= x_coord_tempory[0]; //the first value is the smallest
        x_coord[i] = x_coord_tempory[i]; //i have absolutely noooo idea, why you have to write it, in that way and cant do x_coord = x_coord_temp. Yk, it worked fine before, but yeeah, thats fucking cringe. --> the reason is extremely simple, its because i am writing in js. 
    }

    return [Math.round(temp_max * 100)/100, Math.round(temp_min * 100)/100, Math.round(time_divided * 100)/100]; //returnig the values as a array

    //return{ //this way return my values as a object
    //    temp_max, //saving the values, quite important for the graphical settings
    //    temp_min, //same here
    //    time_divided //saving the data, so that I can use it again
    //};   
}

function sensor_time_calculate_their_Coordinates_24h(x_coord, sensor_time){
    let x_coord_temp = [];

    //the max time value for the sensors -- i concluded, that a day-time zone will be programmed first and the other participants like 2 weeks shown line, etc. will be connected to this 24-hour data
    const time_24h = x_point_length; //24 hour - coordinate
    const time_1h = time_24h / 24; //1 hour - coordinate

    //important for the time show
    let hour_the_chosen = 0;
    let min_the_chosen = 0;
    let sec_the_chosen = 0;

    for (let i = 0; i < sensor_time.length; i++) { //x-coordinates
        let current_time = sensor_time[i].split(":"); //01:10:00 - [h]:[min]:[s]
        let hour_string = current_time[0];
        let min_string = current_time[1];
        let sec_string = current_time[2];

        if (Number(hour_string) >= 10){ //thats good
            hour_the_chosen = hour_string;
        }
        if (Number(min_string) >= 10){
            min_the_chosen = min_string;
        }
        if (Number(sec_string) >= 10){
            sec_the_chosen = sec_string;
        }
        if (Number(hour_string) < 10){
            hour_the_chosen = hour_string.split("0")[1]; //like 01 -->string[1] ... 1
        }
        if (Number(min_string) < 10){
            min_the_chosen = min_string.split("0")[1];
        }
        if (Number(sec_string) < 10){
            sec_the_chosen = sec_string.split("0")[1];
        }

        let calculate_per_1h_from_this = Number(hour_the_chosen) + Number(min_the_chosen/60) + Number(sec_the_chosen/360); //like 12.5h

        const max_time = 24; //24h --> 24:00:00
        const lowest_time = 0; //0h --> 00:00:00

        let dx_time = max_time - calculate_per_1h_from_this;

        switch (dx_time) { // 0 and 24 are not always there
            case 0:
                    x_coord_temp[i] = time_1h * max_time;
                break;
        
            case 24:
                    x_coord_temp[i] = time_1h * lowest_time;
                break;

            default:
                    x_coord_temp[i] = time_1h * calculate_per_1h_from_this;
                break;
        }
        x_coord[i] = x_coord_temp[i]; //i have absolutely noooo idea, why you have to write it, in that way and cant do x_coord = x_coord_temp. Yk, it worked fine before, but yeeah, thats fucking cringe. --> yeah i like i said, because of js.
        //////////////////check////////////////
        ///////////////////////////////////////
        console.log(i + " . " + x_coord_temp[i] + " - X-Values");
        ///////////////////////////////////////
    }
}

//method for all incoming data --> this will be used for the esp
const fetch_ALL_INFO = async(DNS_address, first_address, second_address, macs, datas)=>{ 
    const address = DNS_address; 
    const first = first_address; 
    const second = `${second_address}`; 
    const response = await fetch(`http://${address}/${first}/${second}`,{ //if you want to use the test_send.http, you have use this: `http://${address}/${first}/${second}/test_send.http`
        method: 'GET',
        headers: {
            'Content-Type': 'text/plain'
        }
    });

    if (!response.ok){ //response.ok == 200
        throw new Error('Data was not found');
    }

    //one message will look like that
    //Data:Time:123:Sensorbox1:BMEData:HTUData:TypKData:OzonData

    const userData = await response.text(); //i have to do asyn, because its parsing at the same when receiving the msg
                                            //when a method returns a promise, u have to use "await"
    data_sensor = userData; //saving my data

    console.log(`"Reading Data ... "`);
    console.log("...");
    console.log("...");
    console.log("...");


    //checking
    //////////////////////////////////////////////////////////////////
    console.log(userData);
    //////////////////////////////////////////////////////////////////
}

//method for reading my data from bme, hpp, ...
const fetchUserInfo = async(DNS_address, first_address, second_address, where_is_it_saved, data_sensor, split_sensor_name, data_sensor_no_sym, sensor_split, sensor_time, sensor_data, split_sensor_frame_msg)=>{ 
    const address = DNS_address; //"localhost"; //this ip is only test-wise constructed --> update: now i will change the ip to tesp.ip from the esp --> update: we have implented a dns address for our esp's, so we'll be using those
    const first = first_address; //"Diplomarbeit";
    const second = `${second_address}`; //"Website";
    const response = await fetch(`http://${address}/${first}/${second}/${where_is_it_saved}`,{ //if you want to use the test_send.http, you have use this: `http://${address}/${first}/${second}/test_send.http`
        method: 'GET',
        headers: {
            'Content-Type': 'text/plain'
        }
    });

    if (!response.ok){ //response.ok == 200
        throw new Error('Data was not found');
    }

    const userData = await response.text(); //i have to do asyn, because its parsing at the same when receiving the msg
                                            //when a method returns a promise, u have to use "await"
    data_sensor = userData; //saving my data

    console.log(`"BRACE FOOOR IMPACT - ${where_is_it_saved}"`);
    console.log("!!!!!!!!!!!!!!!!!!");
    console.log("!!!!!!!!!!!!!!!!!!");
    console.log("!!!!!!!!!!!!!!!!!!");
    console.log("!!!!!!!!!!!!!!!!!!");
    console.log("!!!!!!!!!!!!!!!!!!");
    console.log("!!!!!!!!!!!!!!!!!!");
    console.log("!!!!!!!!!!!!!!!!!!");

    //checking
    //////////////////////////////////////////////////////////////////
    //console.log(userData);
    //////////////////////////////////////////////////////////////////

    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //example how we should extract the data -----> Time:11:30:08:BME:-6.16@Time:11:30:38:BME:31.97 
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    //extracting the beginning name of the sent message --> quite important, when I want to read the data
    const BMEmsg = userData.split(`${split_sensor_name}`)[1]?.trim(); //prevents error messages "?" ... plus '...', with that i can even extract " this symbols 
    
    //now i will split all the uneccessary data
    const without_symbols = BMEmsg.split(`"`)[1]?.trim();

    //checking
    //////////////////////////////////////////////////////////////////
    //console.log(BMEmsg + "\n\nsplit BMEmsg"); //it worksssssssssssssss, yeeeaaa men ---> heeeey, i was extremely happy, when it worked, dont u dare to be mad!
    //console.log(without_symbols + "\n\nsplit symbols"); //it worksssssssssssssss, yeeeaaa men
    //////////////////////////////////////////////////////////////////

    //saving without_symbols to a specific sensor save point
    data_sensor_no_sym = without_symbols;

    //first lets take without_symbols and split the data with @
    const split_data = without_symbols.split('@'); 

    //checking
    //////////////////////////////////////////////////////////////////
    //console.log("-------------------------------\n"); ///it workssssss
    //for (let i = 0; i < split_data.length; i++) {
    //    console.log(split_data[i] + "\n\nsplit @ symbols");
    //}
    //console.log("-------------------------------\n");
    //////////////////////////////////////////////////////////////////

    //saving split_data to a specific sensor
    sensor_split = split_data;

    //select time and data
    for (let i = 0; i < split_data.length; i++) {
        const split_again = (split_data[i].split(`${'Time:'}`)[1]?.trim()).split(`${split_sensor_frame_msg}`);
        sensor_time[i] = split_again[0];
        sensor_data[i] = split_again[1];
    }

    //checking
    //////////////////////////////////////////////////////////////////
    //console.log("-------------------------------\n"); ///it workssssss
    //for (let i = 0; i < sensor_data.length; i++) {
    //    console.log(i + " . " + sensor_time[i] + "\ntime");
    //    console.log(i + " . " + sensor_data[i] + "\ndata");
    //}
    //console.log("-------------------------------\n");
    //////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////
    /* This Format should be readable from this website
    server.on("/bme", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", msgBME);
    });
    */
    //////////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////
}
//To do --> csv save and graph save

setInterval(HTTP_READ(), 1000); //yeah i changed, the paramter, to 1 second --> it should work, but we have to test it properly with the esp

setInterval(Read_all(), 1000); //this should read the actual sensor-esp-data