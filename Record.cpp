#include "Record.h"
#include <fstream>
#include <cstring>

void Record::recordIntoBuffer() {
	ffmpegProc = popen(ffmpegCmd.c_str(), "r");

	for (int i=0;keepRecording;++i) {
		if (i>0) bufferIsFull=1;
		for (pos=0;pos<BUFFER_SIZE && keepRecording;++pos) {
			buffer[pos]=fgetc(ffmpegProc);
		}
	}
	pclose(ffmpegProc);
}

Record::Record(int delay, int bitrate) {
	DELAY = delay;
	BITRATE = bitrate;
	BUFFER_SIZE = DELAY * BITRATE * 1000 / 8;

	buffer = new char[BUFFER_SIZE];

	ffmpegCmd = "ffmpeg -nostdin -loglevel panic -f pulse -i default -b:a " + std::to_string(bitrate) + "k -f mp3 pipe: 2>/dev/null &";

	recordingThread = std::thread(&Record::recordIntoBuffer, this);
}

Record::~Record() {
	keepRecording = false;
	recordingThread.join();
	delete[] buffer;
}

char* Record::getTime() {
	time_t now = time(0);
	char* date = ctime(&now);
	for (int i=0;date[i]!='\n';++i) {
		if (date[i] == ' ') date[i]='_';
		if (date[i] == ':') date[i]='-';
	}
	
	return date;
}

void Record::saveRecording() const {
	std::string outputFilename = getTime();
	outputFilename.pop_back();
	outputFilename.append(".mp3");

	std::ofstream outFile;
	outFile.open(outputFilename, std::ios::binary);

	int currPos = pos;

	if (bufferIsFull) {
		for (int i=currPos+1;i<BUFFER_SIZE;++i) {
			outFile.write(buffer+i, 1);
		}
	}

	for (int i=0;i<currPos+1;++i) {
		outFile.write(buffer+i, 1);
	}

	outFile.close();
}
