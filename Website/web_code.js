//for axe naming
let x_whole_axe_length = 0;
let y_whole_axe_length = 0; 
//important for some naming and scaling the graph
let x_axe_length = 0;
let y_axe_length = 0;
//important for the scaling naming of each axes
let y_point_length = 0;
let x_point_length = 0;

//variables for occuring data flow, like bme, hpp, etc.
let data_bme = "";
let data_hpp = "";
let data_ozon = "";
let data_temp = "";

function apply_button(){ //applying the filter options on the graph
    
}

function update(){

}

function setTime() { //from phillip, some parts of it is from me, but it was kinda changed
    var check = false;
    
    try{
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
        
        xhr.onerror = function(){ //on error function, cause those errors are only detectable through these methods
            console.warn("A error happened, duo to: " + xhr.status);
        };

        if (xhr.status == 200){ //200 meaning its okay and successfull
            xhr.send();
        }
    }
    catch(err){ //this catch wont necessary happen, because http request are kinda weird
      alert("It occurs a errors when sending the httprequest, please make sure, you are connected to your device!\n" + err);
    };
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

    //why, you ask. Very simple, because these methods will update the graph x and y axe lines, the comboxes are sometimes bugged, and dont update at the beginning if you dont change the current item. I had several problems with that, maybe i will change this line of code in the future, but now
    time_setting();
    sensor_setting();
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
    y_whole_axe_length = height*0.8; //saving the whole length of the y axe
    y_axe_length = height*0.75; //saving the length for later
    y_point_length = y_axe_length*0.93; //important time data-showing
    //arrow x
    ctx.beginPath();
    ctx.moveTo(width*0.7, 0);
    ctx.lineTo(width*0.65, -height*0.02);
    ctx.lineTo(width*0.65, height*0.02);
    ctx.lineTo(width*0.7, 0);
    ctx.stroke();
    x_whole_axe_length = width*0.7; //saving the whole length of the x axe
    x_axe_length = width*0.65; //saving the length for later
                               //In addition, it depends on the arrow, because the lines of axe (x,y), 
                               //would cross over the arrow, this would look ugly
    x_point_length = x_axe_length*0.98; //important for the sensor data-showing
    setTime(); //here i am sending httrequest, so our esp gets the current time
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

    const x_axe_name_diff = width * 0.005; //dif ... difference
    const height_for_x_lapses = height*0.01;
    switch (zone) { //changes --> before it was x_axe_lenght - 1 in clearRect etc. --> But i found out that it is too much trouble, to implement it in that way all the time. So i gave the time and sensor axes each unique length, so that i have no trouble with line conflictions
        case "2 hours in a row":
            lapse = 2; //1, because of 0 time
            unit = x_point_length / lapse; //for each time lapse

            ctx.clearRect(-unit/2, 1, unit/2 + x_axe_length-1, height_for_x_lapses + 0.02*height + 12); //clearing the axe, so it is cleared
                                                              //we are beginning at 1, because line with
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
                ctx.fillText([i], [i*unit - unit/150], height_for_x_lapses + 0.02*height);
            }
            ctx.font = "20px serif";
            ctx.fillText("x in hours", x_whole_axe_length + x_axe_name_diff,1);
        break;

        case "Today":
            lapse = 24; //1, because of 0 time
            unit = x_point_length / lapse;
            //clearRect(...,...,unit/2 + x_axe_length-1,...) --> +unit/2, because of the x-axe line shift, thats why, okay?
            ctx.clearRect(-unit/2, 1, unit/2 + x_axe_length-1, height_for_x_lapses + 0.02*height + 12); //clearing the axe, so it is cleared
                                                              //we are beginning at 1, because line with
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
                                                              //we are beginning at 1, because line with
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
                                                              //we are beginning at 1, because line with
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

        case "This Week":
            lapse = 7; //1, because of 0 time
            unit = x_point_length / lapse; //for each time lapse

            ctx.clearRect(-unit/2, 1, unit/2 + x_axe_length-1, height_for_x_lapses + 0.02*height + 12); //clearing the axe, so it is cleared
                                                              //we are beginning at 1, because line with
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
                ctx.fillText([i], [i*unit - unit/45], height_for_x_lapses + 0.02*height);
            }
            ctx.font = "20px serif";
            ctx.fillText("x in days", x_whole_axe_length + x_axe_name_diff,1);
        break;

        case "Last Week":
            lapse = 7; //1, because of 0 time
            unit = x_point_length / lapse; //for each time lapse

            ctx.clearRect(-unit/2, 1, unit/2 + x_axe_length-1, height_for_x_lapses + 0.02*height + 12); //clearing the axe, so it is cleared
                                                              //we are beginning at 1, because line with
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
                ctx.fillText([i], [i*unit - unit/45], height_for_x_lapses + 0.02*height);
            }
            ctx.font = "20px serif";
            ctx.fillText("x in days", x_whole_axe_length + x_axe_name_diff,1);
        break;

        case "2 Weeks in a row":
            lapse = 14; //1, because of 0 time
            unit = x_point_length / lapse; //for each time lapse

            ctx.clearRect(-unit/2, 1, unit/2 + x_axe_length-1, height_for_x_lapses + 0.02*height + 12); //clearing the axe, so it is cleared
                                                              //we are beginning at 1, because line with
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
                ctx.fillText([i], [i*unit - unit/10], height_for_x_lapses + 0.02*height);
            }
            ctx.font = "20px serif";
            ctx.fillText("x in days", x_whole_axe_length + x_axe_name_diff,1);
        break;
    }
}

function measure_text_x_axe_and_delete(){ //for x-axe arrow naming
    const canvas = document.getElementById('graph');
    const ctx = canvas.getContext("2d"); //getting the the features of the canvas, so i can promptly edit it AKA the context ... ctx
    
    //getting the width
    const width = canvas.width;

    const x_axe_name_diff = width * 0.005; //dif ... difference

    ctx.lineWidth = 0.5;
    ctx.strokeStyle = '#000000'; //if strokestyle was not set, it will now be set in the occupied function
    
    //getting the measurments - the naming of the x-axe will be measured here
    var x_months_length = ctx.measureText("x in months").width;
    var x_days_length = ctx.measureText("x in days").width;
    var x_hours_length = ctx.measureText("x in hours").width;

    //now deleting
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
    const ctx = canvas.getContext("2d"); //getting the the features of the canvas, so i can promptly edit it AKA the context ... ctx
    
    //getting the width
    const width = canvas.width;

    const y_axe_name_diff = width * 0.005; //diff ... difference

    ctx.lineWidth = 0.5;
    ctx.strokeStyle = '#000000'; //if strokestyle was not set, it will now be set in the occupied function
    
    //getting the measurments - the naming of the x-axe will be measured here
    var y_temp_length = ctx.measureText("Temperature").width;
    var y_airmois_length = ctx.measureText("Airmoisure").width;
    var y_airpress_length = ctx.measureText("Airpressure").width;

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
}

function sensor_setting(){
    //the same like setting_time()
    const sensor = document.getElementById("box_sens").value;
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
            ctx.clearRect(-1, 2, -width*0.1, -y_axe_length-1); // clearrect(..,2,..,...), because if do less than that, we well we will delete the arrow. Why, you ask, very simple cause this a self drawn graph not like any other you find online
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

        case "Airmoisure":
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
                //giving numbers (names) to the lines with corosponding --> -75°C...250°C --> our complete range are 325°C --> meanig per lapse we have --> 32.5 degree difference
                ctx.font = "20px serif";
                const unit_temp = 100/lapse;
                ctx.fillText([(i*unit_temp)] + "%", -width*0.06, -i*unit);
            }
            ctx.font = "20px serif";
            ctx.fillText("Airmoisure", -1, -[y_whole_axe_length + y_axe_name_diff]); //arrow naming
        break;

        case "Airpressure":
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
                //giving numbers (names) to the lines with corosponding --> -75°C...250°C --> our complete range are 325°C --> meanig per lapse we have --> 32.5 degree difference
                ctx.font = "20px serif";
                const unit_temp = 800/lapse; //1100hPa - 300hPa = 800hPa
                ctx.fillText([300 + (i*unit_temp)] + "hPa", -width*0.06, -i*unit);
            }
            ctx.font = "20px serif";
            ctx.fillText("Airpressure", -1, -[y_whole_axe_length + y_axe_name_diff]); //arrow naming
        break;

        case "Gas":
            //i dont know we have to test it first
        break;
    }
}

function HTTP_SET(){ //here i will set and save the values for the sensor data
    //bme should also measure temperature 

    //ozon in ppm, does it change


    //first i will get the generated data, which is 


}

function Read_from_bme(){ //this method will later be implemented in HTTP_SET
    fetchUserInfo(data_bme);
    
    if (data_bme != ""){
        //here i will start to take the data from the string

        //...
    }
}


//method for reading my data from bme, hpp, ...
const fetchUserInfo = async(data_type)=>{ 
    const address = "localhost"; //this ip is only test-wise constructed --> update: now i will change the ip to tesp.ip from the esp
    const first = "Diplomarbeit";
    const second = "Website";
    const response = await fetch(`http://${address}/${first}/${second}/website.html`,{
        method: 'GET',
        headers: {
            'Content-Type': 'text/plain'
        }
    });

    if (!response.ok){ //response.ok == 200
        throw new Error('Data was not found');
    }

    //parse our json

    const userData = await response.text(); //i have to do asyn, because its parsing at the same when receiving the msg
    //when a method returns a promise, u have to use "await"
    data_type = userData; //saving my data
    console.log(userData);
    
    const params = new URLSearchParams(userData);
    const BMEmsg = params.get('msgBME');
    
    console.log(BMEmsg + "yessssssssssssss");

    /* This Format should be readable from this website
    server.on("/bme", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", msgBME);
    });
    */
}

setInterval(Read_from_bme(), 1000); //testing, if my requests are succesfull

//month, years is not important

//1 hour show, 2 days in row

//csv save and graph save