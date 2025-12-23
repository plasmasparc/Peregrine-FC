#include "WebTracker.h"

WebTracker::WebTracker() : server(80) {
    position_count = 0;
    position_index = 0;
}

void WebTracker::init() {
    WiFi.softAP("LoRa_Tracker", "12345678");
    
    server.on("/", [this]() { handleRoot(); });
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

String WebTracker::generateHTML() {
    String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Flight Tracker</title>";
    html += "<style>body{margin:0;font-family:monospace;}#map{width:100vw;height:100vh;}</style></head><body>";
    html += "<div id='map'></div><script src='https://unpkg.com/leaflet@1.9.4/dist/leaflet.js'></script>";
    html += "<link rel='stylesheet' href='https://unpkg.com/leaflet@1.9.4/dist/leaflet.css'/><script>";
    
    if(position_count == 0) {
        html += "var map=L.map('map').setView([47.4979,19.0402],13);";
    } else {
        uint16_t latest = (position_index == 0) ? (position_count - 1) : (position_index - 1);
        html += "var map=L.map('map').setView([" + String(positions[latest].lat, 7) + "," + String(positions[latest].lon, 7) + "],15);";
    }
    
    html += "L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png').addTo(map);";
    
    if(position_count > 0) {
        html += "var coords=[";
        for(uint16_t i = 0; i < position_count; i++) {
            uint16_t idx = (position_index + MAX_POSITIONS - position_count + i) % MAX_POSITIONS;
            html += "[" + String(positions[idx].lat, 7) + "," + String(positions[idx].lon, 7) + "]";
            if(i < position_count - 1) html += ",";
        }
        html += "];L.polyline(coords,{color:'blue',weight:2}).addTo(map);";
        
        uint16_t latest = (position_index == 0) ? (position_count - 1) : (position_index - 1);
        html += "L.circleMarker([" + String(positions[latest].lat, 7) + "," + String(positions[latest].lon, 7);
        html += "],{radius:6,fillColor:'red',color:'red',fillOpacity:1}).addTo(map);";
        
        if(position_count >= 2) {
            uint16_t prev = (latest == 0) ? (position_count - 1) : (latest - 1);
            double dlat = positions[latest].lat - positions[prev].lat;
            double dlon = positions[latest].lon - positions[prev].lon;
            double angle = atan2(dlon, dlat) * 180.0 / PI;
            
            html += "var arrow=L.marker([" + String(positions[latest].lat, 7) + "," + String(positions[latest].lon, 7);
            html += "],{icon:L.divIcon({html:'<div style=\"transform:rotate(" + String(angle, 1);
            html += "deg);font-size:24px;line-height:24px;\">▲</div>',className:'',iconSize:[24,24]})}).addTo(map);";
        }
    }
    
    html += "</script></body></html>";
    return html;
}
