#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

ESP8266WebServer server(80);

// --- Motion Logic Variables ---
const int bufferSize = 30;
int rssiHistory[bufferSize];
int currentIndex = 0;

// The HTML/JavaScript for the Dashboard
const char INDEX_HTML[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <script src="https://code.highcharts.com/highcharts.js"></script>
    <style>
        body { font-family: sans-serif; text-align: center; background: #1a1a1a; color: white; }
        .container { width: 100%; max-width: 600px; margin: auto; }
        h2 { color: #00ff88; }
    </style>
</head>
<body>
    <div class="container">
        <h2>Sovereign Signal System</h2>
        <div id="chart-rssi" class="container"></div>
    </div>
    <script>
        var chart = new Highcharts.Chart({
            chart:{ renderTo:'chart-rssi', backgroundColor: '#1a1a1a' },
            title:{ text: 'Live Radio Shadow' },
            series: [{ showInLegend: false, data: [], color: '#00ff88' }],
            plotOptions: { line: { animation: false, dataLabels: { enabled: false } }, series: { color: '#00ff88' } },
            xAxis: { type: 'datetime', dateTimeLabelFormats: { second: '%H:%M:%S' } },
            yAxis: { title: { text: 'Strength (dBm)' }, gridLineColor: '#333' },
            credits: { enabled: false }
        });
        setInterval(function() {
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function() {
                if (this.readyState == 4 && this.status == 200) {
                    var x = (new Date()).getTime(), y = parseFloat(this.responseText);
                    if(chart.series[0].data.length > 40) { chart.series[0].addPoint([x, y], true, true, true); }
                    else { chart.series[0].addPoint([x, y], true, false, true); }
                }
            };
            xhttp.open("GET", "/rssi", true);
            xhttp.send();
        }, 500); // Update every 500ms
    </script>
</body>
</html>
)=====";

void handleRoot() { server.send(200, "text/html", INDEX_HTML); }

void handleRSSI() {
  server.send(200, "text/plain", String(WiFi.RSSI()));
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  
  server.on("/", handleRoot);
  server.on("/rssi", handleRSSI);
  server.begin();
  
  Serial.println("\n[SYSTEM ONLINE]");
  Serial.print("Access Dashboard at: http://");
  Serial.println(WiFi.localIP());
}

void loop() {
  server.handleClient();
}
