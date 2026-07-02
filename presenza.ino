#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>

const byte DNS_PORT = 53;
DNSServer dnsServer;
WebServer server(80);

const char* ssid = "xxxxxx";//name you want to display
const char* password = "12345678";//password 

const char* adminUser = "admin username";
const char* adminPass = "admin password";

String currentSessionId = "";

struct Student {
  String regNo;
  String timestamp;
};

Student students[200];
int countStudents = 0;

bool isTeacherAuthenticated() {
  return server.authenticate(adminUser, adminPass);
}

bool alreadyMarked(String regNo) {
  for (int i = 0; i < countStudents; i++) {
    if (students[i].regNo == regNo) {
      return true;
    }
  }
  return false;
}

void generateSessionId() {
  randomSeed(analogRead(0));
  long randomNum = random(100000, 999999);
  currentSessionId = "SESS-" + String(randomNum);
}

String attendancePage() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>CampusOS | Attendance Portal</title>
<link href="https://fonts.googleapis.com/css2?family=Nunito:wght@400;600;700;900&display=swap" rel="stylesheet">
<style>
  *, *::before, *::after { box-sizing: border-box; margin: 0; padding: 0; }
  body { font-family: 'Nunito', sans-serif; background: #f0f4f8; display: flex; align-items: center; justify-content: center; min-height: 100vh; padding: 20px; }
  .wrapper { width: 100%; max-width: 420px; text-align: center; }
  .branding { margin-bottom: 32px; display: flex; flex-direction: column; align-items: center; }
  .logo-img { height: 75px; width: auto; object-fit: contain; margin-bottom: 14px; filter: drop-shadow(0 4px 8px rgba(0,0,0,0.05)); }
  .brand-title { color: #1a1a2e; font-weight: 900; font-size: 24px; letter-spacing: -0.5px; }
  .brand-sub { color: #657786; font-size: 13px; margin-top: 4px; }
  .card { background: #ffffff; border-radius: 14px; padding: 32px; box-shadow: 0 4px 24px rgba(0,0,0,0.06); border: 1px solid rgba(0,48,135,0.05); text-align: left; animation: slideUp 0.3s ease; }
  @keyframes slideUp { from { opacity: 0; transform: translateY(12px); } to { opacity: 1; transform: translateY(0); } }
  .card-title { color: #003087; font-weight: 900; font-size: 20px; margin-bottom: 6px; }
  .card-desc { color: #888; font-size: 13px; margin-bottom: 24px; }
  .session-container { display: flex; align-items: center; justify-content: space-between; background: #f8fafc; border: 1px solid #e2e8f0; border-radius: 10px; padding: 10px 14px; margin-bottom: 20px; }
  .session-lbl { font-size: 11px; font-weight: 700; color: #64748b; text-transform: uppercase; }
  .session-val { font-size: 13px; font-weight: 900; color: #e87722; }
  .field-group { margin-bottom: 20px; }
  .field-lbl { display: block; font-size: 13px; font-weight: 700; color: #475569; margin-bottom: 6px; }
  .f-input { width: 100%; padding: 12px 14px; border: 1.5px solid #dde3ed; border-radius: 10px; font-size: 15px; font-family: inherit; outline: none; transition: border-color 0.2s; background: #fff; color: #1e293b; }
  .f-input:focus { border-color: #003087; }
  .btn { width: 100%; border: none; border-radius: 10px; padding: 14px; font-weight: 700; font-size: 16px; font-family: inherit; cursor: pointer; transition: opacity 0.2s; margin-top: 8px; text-align: center; }
  .btn:hover { opacity: 0.9; }
  .btn-blue { background: #003087; color: #fff; }
</style>
</head>
<body>
<div class="wrapper">
  <div class="branding">
    <img class="logo-img" src="https://www.ritchennai.org/images/logo.png" alt="RIT Logo">
    <div class="brand-title">CampusOS</div>
    <div class="brand-sub">Rajalakshmi Institute of Technology</div>
  </div>
  <div class="card">
    <h2 class="card-title">Check In</h2>
    <p class="card-desc">Verify your presence for today's lecture roster.</p>
    <div class="session-container">
      <span class="session-lbl">Roster Session</span>
      <span class="session-val">)rawliteral";
  html += currentSessionId;
  html += R"rawliteral(</span>
    </div>
    <form action="/submit" method="POST">
      <div class="field-group">
        <label class="field-lbl">Register Number</label>
        <input type="tel" name="regno" class="f-input" placeholder="e.g., 211501098" inputmode="numeric" pattern="[0-9]*" required>
      </div>
      <button type="submit" class="btn btn-blue">Mark Attendance</button>
    </form>
  </div>
</div>
</body>
</html>
)rawliteral";
  return html;
}

void handleRoot() {
  server.send(200, "text/html", attendancePage());
}

void handleSubmit() {
  String regNo = server.arg("regno");
  regNo.trim();

  String templateHead = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>CampusOS | Update</title>
<link href="https://fonts.googleapis.com/css2?family=Nunito:wght@400;600;700;900&display=swap" rel="stylesheet">
<style>
  body { font-family: 'Nunito', sans-serif; background: #f0f4f8; display: flex; align-items: center; justify-content: center; min-height: 100vh; padding: 20px; margin: 0; }
  .card { background: #fff; border-radius: 14px; padding: 32px; box-shadow: 0 4px 24px rgba(0,0,0,0.06); max-width: 400px; width: 100%; text-align: center; animation: slideUp 0.3s ease; }
  @keyframes slideUp { from { opacity: 0; transform: translateY(12px); } to { opacity: 1; transform: translateY(0); } }
  .icon { font-size: 48px; margin-bottom: 16px; }
  h2 { color: #003087; font-weight: 900; font-size: 20px; margin-bottom: 12px; }
  p { color: #64748b; font-size: 14px; margin-bottom: 8px; text-align: left; background: #f8fafc; padding: 10px 14px; border-radius: 8px; border: 1px solid #e2e8f0; }
  .btn { display: inline-block; width: 100%; border: none; border-radius: 10px; padding: 12px; font-weight: 700; font-size: 14px; font-family: inherit; cursor: pointer; text-decoration: none; margin-top: 16px; transition: opacity 0.2s; }
  .btn-blue { background: #003087; color: #fff; }
  .btn-gray { background: #e2e8f0; color: #475569; }
</style>
</head>
<body>
<div class="card">
)rawliteral";

  // Pure digits verification
  for (int i = 0; i < regNo.length(); i++) {
    if (!isDigit(regNo.charAt(i))) {
      String response = templateHead + 
        "<div class='icon'>❌</div>"
        "<h2>Invalid Registration Format</h2>"
        "<div style='margin-top:14px; margin-bottom:6px;'><p><strong>Error:</strong> The system detected non-numeric elements. Registration mapping fields accept integer records only.</p></div>"
        "<a href='/' class='btn btn-gray'>Return to Form</a>"
        "</div></body></html>";
      server.send(200, "text/html", response);
      return;
    }
  }

  if (alreadyMarked(regNo)) {
    String response = templateHead + 
      "<div class='icon'>⚠️</div>"
      "<h2>Submission Denied</h2>"
      "<div style='margin-top:14px; margin-bottom:6px;'><p><strong>Status:</strong> Identity already logged. This registration number has verified metrics within this active pool.</p></div>"
      "<a href='/' class='btn btn-gray'>Return to Form</a>"
      "</div></body></html>";
    server.send(200, "text/html", response);
    return;
  }

  if (countStudents >= 200) {
    String response = templateHead + 
      "<div class='icon'>❌</div>"
      "<h2>Roster Limit Exceeded</h2>"
      "<div style='margin-top:14px; margin-bottom:6px;'><p><strong>Error:</strong> Local memory array allocation ceiling breached.</p></div>"
      "</div></body></html>";
    server.send(200, "text/html", response);
    return;
  }

  unsigned long totalSeconds = millis() / 1000;
  int minutes = totalSeconds / 60;
  int seconds = totalSeconds % 60;
  String timeStr = String(minutes) + "m " + String(seconds) + "s";

  students[countStudents].regNo = regNo;
  students[countStudents].timestamp = timeStr;
  countStudents++;

  String response = templateHead + 
    "<div class='icon' style='color:#2ecc71'>✅</div>"
    "<h2>Identity Checked In</h2>"
    "<div style='margin-top:14px; margin-bottom:6px;'>"
    "<p><strong>Session:</strong> " + currentSessionId + "</p>"
    "<p><strong>Registration:</strong> " + regNo + "</p>"
    "</div>"
    "<a href='/' class='btn btn-blue'>Acknowledge</a>"
    "</div></body></html>";

  server.send(200, "text/html", response);

  Serial.println("SYS:LOG:" + currentSessionId + ":" + regNo);
}

void handleList() {
  if (!isTeacherAuthenticated()) {
    return server.requestAuthentication();
  }

  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>CampusOS | Faculty Deck</title>
<link href="https://fonts.googleapis.com/css2?family=Nunito:wght@400;600;700;900&display=swap" rel="stylesheet">
<style>
  *, *::before, *::after { box-sizing: border-box; margin: 0; padding: 0; }
  body { font-family: 'Nunito', sans-serif; background: #f0f4f8; padding: 24px; color: #1e293b; }
  .header-container { max-width: 700px; margin: 0 auto 24px; display: flex; align-items: center; justify-content: space-between; flex-wrap: wrap; gap: 16px; }
  h2 { color: #003087; font-weight: 900; font-size: 24px; }
  .meta-pills { display: flex; gap: 8px; font-size: 13px; font-weight: 700; color: #475569; margin-top: 4px; }
  .pill { background: #fff; padding: 4px 12px; border-radius: 99px; border: 1px solid #e2e8f0; box-shadow: 0 1px 3px rgba(0,0,0,0.02); }
  .btn { border: none; border-radius: 10px; padding: 10px 20px; font-weight: 700; cursor: pointer; font-size: 14px; font-family: inherit; transition: opacity 0.2s; text-decoration: none; display: inline-block; }
  .btn:hover { opacity: 0.9; }
  .btn-orange { background: #e87722; color: #fff; }
  .table-card { max-width: 700px; margin: 0 auto; background: #fff; border-radius: 14px; box-shadow: 0 4px 24px rgba(0,0,0,0.05); border: 1px solid rgba(0,48,135,0.05); overflow: hidden; }
  .tbl { width: 100%; border-collapse: collapse; text-align: left; }
  .tbl th { background: #003087; color: #fff; padding: 14px 18px; font-size: 13px; font-weight: 700; text-transform: uppercase; letter-spacing: 0.5px; }
  .tbl td { padding: 14px 18px; border-bottom: 1px solid #f0f4f8; font-size: 14px; color: #334155; }
  .tbl tr:last-child td { border-bottom: none; }
  .tbl tr:hover td { background: #f8fafc; }
  .empty-state { text-align: center; padding: 48px; color: #94a3b8; font-weight: 600; }
  @media(max-width: 767px) { body { padding: 14px; } .tbl th, .tbl td { padding: 10px 12px; font-size: 12px; } .header-container { flex-direction: column; align-items: flex-start; gap: 12px; } .btn { width: 100%; text-align: center; } }
</style>
</head>
<body>
<div class="header-container">
  <div>
    <h2>Faculty Management Dashboard</h2>
    <div class="meta-pills">
)rawliteral";

  html += "<span class='pill'>Session: " + currentSessionId + "</span>";
  html += "<span class='pill'>Roster Size: " + String(countStudents) + "</span>";
  html += R"rawliteral(
    </div>
  </div>
  <a href="/export" class="btn btn-orange">📥 Export CSV Dataset</a>
</div>
<div class="table-card">
)rawliteral";

  if (countStudents == 0) {
    html += "<div class='empty-state'>No student telemetry captured in this workspace pool.</div>";
  } else {
    html += "<table class='tbl'><thead><tr><th>S.No</th><th>Register No</th><th>Timestamp (Boot)</th></tr></thead><tbody>";
    for (int i = 0; i < countStudents; i++) {
      html += "<tr><td>" + String(i + 1) + "</td><td style='font-weight:600; color:#003087;'>" 
              + students[i].regNo + "</td><td style='color:#64748b; font-size:13px;'>" 
              + students[i].timestamp + "</td></tr>";
    }
    html += "</tbody></table>";
  }

  html += "</div></body></html>";
  server.send(200, "text/html", html);
}

void handleExport() {
  if (!isTeacherAuthenticated()) {
    return server.requestAuthentication();
  }

  server.sendHeader("Content-Disposition", "attachment; filename=Roster_" + currentSessionId + ".csv");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/csv", "");

  server.sendContent("S.No,Session ID,Register Number,Timestamp\n");

  for (int i = 0; i < countStudents; i++) {
    String csvLine = String(i + 1) + "," 
                   + currentSessionId + "," 
                   + students[i].regNo + "," 
                   + students[i].timestamp + "\n";
    server.sendContent(csvLine);
  }
}

void handleNotFound() {
  server.sendHeader("Location", "/", true);
  server.send(302, "text/plain", "");
}

void setup() {
  Serial.begin(115200);
  generateSessionId();

  WiFi.softAP(ssid, password);

  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/submit", HTTP_POST, handleSubmit);
  server.on("/list", handleList);
  server.on("/export", handleExport);
  server.onNotFound(handleNotFound);
  
  server.begin();
}

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();
}