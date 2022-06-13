//import dgram from 'dgram';
import express from 'express';
import { createWriteStream, existsSync, readdirSync } from 'fs';
import path from 'path';

//const MULTICAST_IP = '239.255.32.16'

const app = express()

app.use(express.static('public', {extensions: ['html']}))

app.get('/api/sensors/firmware', (req, resp) => {
	let version = req.query['version']
	if(version && (!/^[0-9]+$/.test(version) || isNaN(parseInt(version)) || !existsSync(`firmwares/${version}`))) {
		resp.sendStatus(400)
		
		return
	}
	
	if(!version) {
		let latestVersion = -1
		
		for(let file of readdirSync('firmwares')) {
			let version
			if(/^[0-9]+$/.test(file) && !isNaN(version = parseInt(file))) {
				if(latestVersion < version) latestVersion = version
			}
		}
		
		if(latestVersion < 0) {
			resp.sendStatus(500)
			
			return
		}
		
		version = latestVersion
	}
	
	resp.sendFile(path.resolve(`firmwares/${version}`))
})

app.get('/api/sensors/check_update', (req, resp) => {
	let latestVersion = -1
	
	for(let file of readdirSync('firmwares')) {
		let version
		if(/^[0-9]+$/.test(file) && !isNaN(version = parseInt(file))) {
			if(latestVersion < version) latestVersion = version
		}
	}
	
	if(latestVersion < 0) {
		resp.sendStatus(500)
		
		return
	}
	
	let version = req.query['version']
	if(typeof version !== 'number') version = parseInt(version)
	
	if(isNaN(version) || version < latestVersion) resp.send('1')
	else resp.send('0')
})

app.post('/admin/firmware', (req, resp) => {
	let version = req.query['version']
	if(!/^[0-9]+$/.test(version) || isNaN(parseInt(version))) {
		resp.sendStatus(400)
		
		return
	}
	
	const stream = createWriteStream(`firmwares/${version}`)
	
	req.on('data', data => {
		stream.write(data)
	})
	
	req.on('end', () => {
		stream.close()
	})
	
	resp.sendStatus(200)
})

app.listen(80)

/*const multicast = dgram.createSocket('udp4')

multicast.on('listening', () => {
	multicast.setBroadcast(true)
	multicast.setMulticastTTL(128)
	multicast.addMembership(MULTICAST_IP)
})

multicast.on('message', (msg, src) => {
	console.log('Message')
})

multicast.bind(1337)*/