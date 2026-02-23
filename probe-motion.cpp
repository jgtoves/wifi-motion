#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// --- Configuration ---
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

ESP8266WebServer server(80);
volatile int lastSniffedRSSI = -100;

// This function intercepts raw packets from the air
void sniffer(uint8_t *buf, uint16_t len) {
  // We extract the RSSI from the raw radio header
  int8_t rssi = (int8_t)buf[0]; 
  // We only track strong signals (likely your immediate neighbors/perimeter)
  if (rssi > -70) {
    lastSniffedRSSI = rssi;
  }
}

// The Dashboard HTML (optimized for 'Passive' viewing)
const char INDEX_HTML[] PROGMEM = R"=====(
<!DOCTYPE html><html><head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<script src="https://code.highcharts.com/highcharts.js"></script>
<style>body{background:#000;color:#0f0;font-family:monospace;text-align:center;}</style>
</head><body>
<h2>PASSIVE PROBE RADAR</h2>
<div id="chart-rssi" style="height:300px;"></div>
<script>
var chart = new Highcharts.Chart({
    chart:{renderTo:'chart-rssi',backgroundColor:'#000'},
    title:{text:'External Signal Attenuation',style:{color:'#0f0'}},
    series:[{data:[],color:'#0f0',name:'Neighbor Signal'}],
    xAxis:{type:'datetime'},yAxis:{title:{text:'dBm'}},
    credits:{enabled:false}
});
setInterval(function(){
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function(){
        if(this.readyState==4 && this.status==200){
            var x=(new Date()).getTime(), y=parseFloat(this.responseText);
            chart.series[0].addPoint([x,y],true,chart.series[0].data.length>50);
        }
    };
    xhttp.open("GET","/data",true);xhttp.send();
},300);
</script></body></html>
)=====";

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_AP_STA); // Hybrid mode: Connect to your phone + Sniff others
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); }

  server.on("/", [](){ server.send(200, "text/html", INDEX_HTML); });
  server.on("/data", [](){ server.send(200, "text/plain", String(lastSniffedRSSI)); });
  server.begin();

  // Enable the Sniffer 'Ear'
  wifi_promiscuous_enable(1);
  wifi_set_promiscuous_rx_cb(sniffer);
}

void loop() {
  server.handleClient();
  // We stay on one channel for stability, or you can rotate to 'Sweep' the air
  wifi_set_channel(1); 
}
