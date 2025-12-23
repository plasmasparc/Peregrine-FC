#include "WebTracker.h"

WebTracker::WebTracker() : server(80) {
    position_count = 0;
    position_index = 0;
}

void WebTracker::init() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    while(WiFi.status() != WL_CONNECTED) {
        delay(500);
    }
    
    server.on("/", [this]() { handleRoot(); });
    server.on("/data", [this]() { handleData(); });
    server.begin();
}

void WebTracker::addPosition(double lat, double lon, float roll, float pitch) {
    positions[position_index].lat = lat;
    positions[position_index].lon = lon;
    positions[position_index].roll = roll;
    positions[position_index].pitch = pitch;
    positions[position_index].timestamp_ms = millis();
    
    position_index = (position_index + 1) % MAX_POSITIONS;
    if(position_count < MAX_POSITIONS) position_count++;
}

void WebTracker::handleClient() {
    server.handleClient();
}

void WebTracker::handleRoot() {
    server.send(200, "text/html", generateHTML());
}

void WebTracker::handleData() {
    String json = "{";
    json += "\"count\":" + String(position_count) + ",";
    
    if(position_count > 0) {
        uint16_t latest = (position_index == 0) ? (position_count - 1) : (position_index - 1);
        json += "\"lat\":" + String(positions[latest].lat, 7) + ",";
        json += "\"lon\":" + String(positions[latest].lon, 7) + ",";
        json += "\"roll\":" + String(positions[latest].roll, 1) + ",";
        json += "\"pitch\":" + String(positions[latest].pitch, 1) + ",";
        
        json += "\"coords\":[";
        for(uint16_t i = 0; i < position_count; i++) {
            uint16_t idx = (position_index + MAX_POSITIONS - position_count + i) % MAX_POSITIONS;
            json += "[" + String(positions[idx].lat, 7) + "," + String(positions[idx].lon, 7) + "]";
            if(i < position_count - 1) json += ",";
        }
        json += "],";
        
        if(position_count >= 2) {
            uint16_t prev = (latest == 0) ? (position_count - 1) : (latest - 1);
            double dlat = positions[latest].lat - positions[prev].lat;
            double dlon = positions[latest].lon - positions[prev].lon;
            double angle = atan2(dlon, dlat) * 180.0 / PI;
            json += "\"angle\":" + String(angle, 1);
        } else {
            json += "\"angle\":0";
        }
    }
    
    json += "}";
    server.send(200, "application/json", json);
}

String WebTracker::generateHTML() {
    String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Flight Tracker</title>";
    html += "<style>body{margin:0;font-family:monospace;}#map{width:70vw;height:100vh;float:left;}#horizon{width:30vw;height:100vh;float:right;background:#000;position:relative;display:flex;align-items:center;justify-content:center;}</style></head><body>";
    html += "<div id='map'></div><div id='horizon'><canvas id='ahi' width='400' height='400'></canvas></div>";
    html += "<script src='https://unpkg.com/leaflet@1.9.4/dist/leaflet.js'></script>";
    html += "<link rel='stylesheet' href='https://unpkg.com/leaflet@1.9.4/dist/leaflet.css'/><script>";
    html += "var map=L.map('map').setView([47.4979,19.0402],13);";
    html += "L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png').addTo(map);";
    html += "var polyline=null;var marker=null;var arrow=null;var mapInitialized=false;";
    html += "var canvas=document.getElementById('ahi');var ctx=canvas.getContext('2d');";
    html += "function drawHorizon(roll,pitch){ctx.clearRect(0,0,400,400);";
    html += "ctx.save();ctx.beginPath();ctx.arc(200,200,180,0,2*Math.PI);ctx.clip();";
    html += "ctx.translate(200,200);ctx.rotate(-roll*Math.PI/180);ctx.translate(0,pitch*3);";
    html += "ctx.fillStyle='#87CEEB';ctx.fillRect(-400,-400,800,400);";
    html += "ctx.fillStyle='#8B4513';ctx.fillRect(-400,0,800,400);";
    html += "ctx.strokeStyle='#FFF';ctx.lineWidth=3;ctx.beginPath();ctx.moveTo(-400,0);ctx.lineTo(400,0);ctx.stroke();";
    html += "for(var i=-30;i<=30;i+=10){if(i!=0){ctx.beginPath();ctx.moveTo(-30,i*3);ctx.lineTo(30,i*3);ctx.stroke();}}";
    html += "ctx.restore();";
    html += "ctx.strokeStyle='#FFF';ctx.lineWidth=2;ctx.beginPath();ctx.arc(200,200,180,0,2*Math.PI);ctx.stroke();";
    html += "ctx.strokeStyle='#FF0';ctx.lineWidth=3;";
    html += "ctx.beginPath();ctx.moveTo(140,200);ctx.lineTo(180,200);ctx.stroke();";
    html += "ctx.beginPath();ctx.moveTo(220,200);ctx.lineTo(260,200);ctx.stroke();";
    html += "ctx.beginPath();ctx.moveTo(200,210);ctx.lineTo(195,200);ctx.lineTo(200,195);ctx.lineTo(205,200);ctx.closePath();ctx.stroke();}";
    html += "function update(){fetch('/data').then(r=>r.json()).then(d=>{";
    html += "if(d.count>0){";
    html += "if(polyline)map.removeLayer(polyline);if(marker)map.removeLayer(marker);if(arrow)map.removeLayer(arrow);";
    html += "polyline=L.polyline(d.coords,{color:'blue',weight:2}).addTo(map);";
    html += "marker=L.circleMarker([d.lat,d.lon],{radius:6,fillColor:'red',color:'red',fillOpacity:1}).addTo(map);";
    html += "if(!mapInitialized){map.setView([d.lat,d.lon],15);mapInitialized=true;}";
    html += "if(d.count>=2){";
    html += "arrow=L.marker([d.lat,d.lon],{icon:L.divIcon({html:'<div style=\"transform:rotate('+d.angle+'deg);font-size:24px;line-height:24px;\">▲</div>',className:'',iconSize:[24,24]})}).addTo(map);";
    html += "}drawHorizon(d.roll,d.pitch);};});}";
    html += "setInterval(update,200);update();";
    html += "</script></body></html>";
    return html;
}

String WebTracker::getIPAddress() {
    return WiFi.localIP().toString();
}
