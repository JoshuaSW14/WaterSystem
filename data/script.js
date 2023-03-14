function convertRange(value, r1, r2) {
  return ((value - r1[0]) * (r2[1] - r2[0])) / (r1[1] - r1[0]) + r2[0];
}

function waterPlant(){
  var xmlHttp = new XMLHttpRequest();

  xmlHttp.open("POST", "/plant", true);
  xmlHttp.setRequestHeader("Content-Type", "application/x-www-form-urlencoded");

  xmlHttp.onreadystatechange = () => {
    if (xmlHttp.readyState === XMLHttpRequest.DONE && XPathResult.status === 200){
      console.log("Plant = Water sending to esp32");
    }
  }

  xmlHttp.send("plant=water");
  console.log("sent!");
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
        convertRange(espData.moisture, [10, 30], [0, 100]) + "%";
      document.getElementById("moisture").innerHTML = espData.moisture + "%";

      document.getElementById("temperature").style.width =
        convertRange(espData.temperature, [10, 30], [0, 100]) + "%";
      document.getElementById("temperature").innerHTML = espData.temperature + "°C";

      document.getElementById("humidity").style.width =
        convertRange(espData.humidity, [10, 30], [0, 100]) + "%";
      document.getElementById("humidity").innerHTML = espData.humidity + "%";

      document.getElementById("sunlight").style.width =
        convertRange(espData.sunlight, [10, 30], [0, 100]) + "%";
      document.getElementById("sunlight").innerHTML = espData.sunlight + "%";

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
