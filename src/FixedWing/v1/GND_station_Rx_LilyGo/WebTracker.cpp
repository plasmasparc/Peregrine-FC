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

void WebTracker::addPosition(double lat, double lon) {
    positions[position_index].lat = lat;
    positions[position_index].lon = lon;
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
    html += "<style>body{margin:0;font-family:monospace;}#map{width:100vw;height:100vh;}</style></head><body>";
    html += "<div id='map'></div><script src='https://unpkg.com/leaflet@1.9.4/dist/leaflet.js'></script>";
    html += "<link rel='stylesheet' href='https://unpkg.com/leaflet@1.9.4/dist/leaflet.css'/><script>";
    html += "var map=L.map('map').setView([47.4979,19.0402],13);";
    html += "L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png').addTo(map);";
    html += "var polyline=null;var marker=null;var arrow=null;";
    html += "function update(){fetch('/data').then(r=>r.json()).then(d=>{";
    html += "if(d.count>0){";
    html += "if(polyline)map.removeLayer(polyline);if(marker)map.removeLayer(marker);if(arrow)map.removeLayer(arrow);";
    html += "polyline=L.polyline(d.coords,{color:'blue',weight:2}).addTo(map);";
    html += "marker=L.circleMarker([d.lat,d.lon],{radius:6,fillColor:'red',color:'red',fillOpacity:1}).addTo(map);";
    html += "map.setView([d.lat,d.lon],15);";
    html += "if(d.count>=2){";
    html += "arrow=L.marker([d.lat,d.lon],{icon:L.divIcon({html:'<div style=\"transform:rotate('+d.angle+'deg);font-size:24px;line-height:24px;\">▲</div>',className:'',iconSize:[24,24]})}).addTo(map);";
    html += "}};});}";
    html += "setInterval(update,200);update();";
    html += "</script></body></html>";
    return html;
}

String WebTracker::getIPAddress() {
    return WiFi.localIP().toString();
}
