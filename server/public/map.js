function initMap() {
  var map = L.map('map',{scrollWheelZoom:"center"}).setView([16.82, 96.159], 13);
  
  const markerCoordinates = [
    [16.82, 96.159], 
    // [51.55, -0.09], 
    // [51.58, -0.09], 
  ];

  markerCoordinates.forEach(coord => {
    L.marker(coord).addTo(map);
  });



  // var popup = L.popup()
  //   .setLatLng([51.5, -0.09])
  //   .setContent("Metal Detected Here!")
  //   .openOn(map);

  L.tileLayer('https://tile.openstreetmap.org/{z}/{x}/{y}.png', {
    maxZoom: 19,
    attribution: '© OpenStreetMap'
}).addTo(map);
}

document.addEventListener('DOMContentLoaded', initMap);