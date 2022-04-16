import express from 'express';
import path from 'path';

const app = express()

app.use((req, _, next) => {
	console.log(`${req.ip} -> ${req.path}`)
	next()
})

app.get('/arduino.bin', (_, resp) => {
	resp.sendFile(path.resolve('firmware.bin'))
})

app.get('/check_update', (req, resp) => {
	const latestVersion = 3
	
	let version = req.query['version']
	if(typeof version !== 'number') version = parseInt(version)
	
	if(isNaN(version) || version < latestVersion) resp.send('1')
	else resp.send('0')
})

app.listen(80)