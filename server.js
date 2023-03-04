const http = require('http'); 
const express = require('express');
const app = express();
const fs = require('fs');
app.engine('html',require('ejs').renderFile);

const bodyParser = require('body-parser');  

const host = 'localhost';
const port = 8000;

app.use(bodyParser.urlencoded({ extended: true }));  
app.use(express.static('public'));


const mqtt = require('mqtt')
const client  = mqtt.connect("mqtt://<IP>")


var temp = String(25);

app.get('/input', function(req, res){
    res.render(__dirname + "/" + "page.html", {intemp: temp, crtemp: current});
    client.publish('output', temp);
});


app.post('/get', function (req, res) {
    temp = String(req.body.temp);
    res.render(__dirname + "/" + "page.html", {intemp: temp, crtemp: current});
    console.log("input: ", temp);
    client.publish('output', temp);
});


client.on('connect', function () {
  console.log("MQTT conneceted");
  client.subscribe('output');
  client.subscribe('current');

  client.on('message', function(topic, message){
    if (topic == 'output'){
      console.log("input: ", temp);
    }
    if (topic == 'current'){
      current = message.toString();
      console.log("current: ", current);
    }
  })
});

app.listen(port, host, () => {
  console.log(`Server is running on http://${host}:${port}`);
});

//http://localhost:8000/input
