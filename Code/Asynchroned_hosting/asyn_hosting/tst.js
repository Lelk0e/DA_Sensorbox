window.addEventListener('load', getReadings);

// Get current sensor readings when the page loads  
let button_check = document.getElementById("but");
let hppHum = document.getElementById("humidity");

function getTimeOnLabel(){ // <--- thats working
  let date = new Date(); //DateTime ---> setting here --> we want the actual time everytime, so yeah
  let label_time = document.getElementById("clk");
  label_time.innerText = date.getUTCHours() + "h:" + date.getUTCMinutes() + "min:" +  date.getUTCSeconds() + "s:" + date.getUTCMilliseconds() +  "ms|" + date.getUTCDay() + "." + date.getUTCMonth() + "." + date.getUTCFullYear();
  //this time is utc, meaning on the graphical way you have add 1h, to our austria time zone, then its synchronized!!
}

// Get current sensor readings when the page loads  <----------- not working, because of this: 
// /C:/readings:1 Failed to load resource: net::ERR_FAILED
// Access to XMLHttpRequest at 'file:///C:/readings' from origin 'null' has been blocked by CORS policy: Cross origin requests are only supported for protocol schemes: chrome, chrome-extension, chrome-untrusted, data, http, https, isolated-app
// Access to resource at 'file:///C:/events' from origin 'null' has been blocked by CORS policy: Cross origin requests are only supported for protocol schemes: chrome, chrome-extension, chrome-untrusted, data, http, https, isolated-app.
// <---- when esp ist not connected

// Function to get current readings on the webpage when it loads for the first time
function getReadings(){
  var xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function() {
    if (this.readyState == 4){
      if (this.status == 200) {
        try
        {
          var myObj = JSON.parse(this.responseText);
          console.log(myObj);
          var hum = myObj.humidity;

          if (hum == null){
            hum = "N/A";
          }

          hppHum.innerText = hum;
  
          //this line code updates the time, which will be shown on the following label
          getTimeOnLabel();
        } 
        catch(err)
        {
          console.error("Error parsing /readings response:", err);
          hppHum.innerText = "Error";
        }      
      }
      else{
        //if there is no data or a error
        console.warn("Failed to fetch /readings");
        hppHum.innerText = null;
      }
    }
  }; 
  xhr.open("GET", "mock_data.json", true); // before ... xhr.open("GET", "/readings", true);
  xhr.send();
}

if (!!window.EventSource) {
  var source = new EventSource('/events');
  
  source.addEventListener('open', function(e) {
    console.log("Events Connected");
  }, false);

  source.addEventListener('error', function(e) {
    console.error("Server-Sent-Event Error", e);
    if (e.target.readyState != EventSource.OPEN) {
      console.log("Events Disconnected");
      hppHum.innerText = "Disconnected";
    }
  }, false);
  
  source.addEventListener('message', function(e) {
    try
    {
      let data = JSON.parse(e.data);
      console.log("message successfull", data);
    }
    catch(err)
    {
      console.error("Error parsing message not possible", err)
    }

  }, false);
  
  source.addEventListener('new_readings', function(e) {
    try
    {
      console.log("new_readings", e.data);
      var myObj = JSON.parse(e.data);
      console.log(myObj);
      hppHum.innerText = myObj.humidity;

      if (myObj.humidity == null)
      {
        hum = "N/A";
      }

      //this line code updates the time, which will be shown on the following label
      getTimeOnLabel();
    } catch(err){
      console.error("Error parsing event data:", err);
      hppHum.innerText = null;
    } 
    
  }, false);
}