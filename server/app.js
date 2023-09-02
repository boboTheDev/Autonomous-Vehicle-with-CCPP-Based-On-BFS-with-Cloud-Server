const express = require('express');
const app = express();
const http = require('http').createServer(app);
const io = require('socket.io')(http);
const bodyParser = require('body-parser');

// hostname -I

app.use(express.static('public'));
app.use(express.json({ limit: "10kb" }));

const PORT = 3000;
http.listen(PORT, () => {
  console.log(`Server is running on http://localhost:${PORT}`);
});


io.on('connection', (socket) => {
  console.log('A client connected.');

  socket.on('update', (data) => {
    console.log('Received update:', data);
    io.emit('update', data);
  });

  socket.on('disconnect', () => {
    console.log('A client disconnected.');
  });
});

let a = "Initial Value";

app.post('/updategrid', (req, res) => {

  console.log(req.body);
  // let grid = JSON.parse(req.body.grid);
  // console.log(grid);
  let grid = [];
  let reqStrGridArr = req.body.grid.split(",");

  console.log(reqStrGridArr[0]);


  let gridIndex = 0;
  for (let i=0;i<4;i++){
    let rowArr = [];
    for (let j=0;j<4;j++){
      rowArr.push(reqStrGridArr[gridIndex]);
      gridIndex += 1;
    }
    grid.push(rowArr);
  }
  console.log(grid);

  // console.log(req.body.location);

  // updateGrid(JSON.parse(req.body.grid));


  a = "Welcome"; 
  io.emit('updategrid', { gridData: grid });
  res.send('Variable "a" has been updated.');
});

