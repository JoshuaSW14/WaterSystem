function convertRange(value, r1, r2) {
  return ((value - r1[0]) * (r2[1] - r2[0])) / (r1[1] - r1[0]) + r2[0];
}

function fetchData() {
  var xmlHttp = new XMLHttpRequest();
  xmlHttp.onload = function () {
    if (xmlHttp.status != 200) {
      console.log(
        "GET ERROR: [" + xmlHttp.status + "] " + xmlHttp.responseText
      );
    } else {
      var espData = JSON.parse(xmlHttp.responseText);
      var x = espData.time;

      document.getElementById("moisture").style.width =
        convertRange(espData.temperature, [10, 30], [0, 100]) + "%";
      document.getElementById("moisture").innerHTML = espData.temperature + "%";

      window.setTimeout(fetchData, 5000);
    }
  };
  xmlHttp.onerror = function (e) {
    console.log("ERROR: [" + xmlHttp.status + "] " + xmlHttp.responseText);
    window.setTimeout(fetchData, 5000);
  };
  xmlHttp.open("GET", "/checkInput", true);
  xmlHttp.send();
}

fetchData();
