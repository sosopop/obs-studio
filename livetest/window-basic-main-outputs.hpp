#pragma once

#include <string>

class MainWindow;

struct BasicOutputHandler {
	OBSOutput fileOutput;
	OBSOutput streamOutput;
	OBSOutput replayBuffer;
	bool streamingActive = false;
	bool recordingActive = false;
	bool delayActive = false;
	bool replayBufferActive = false;
	MainWindow *main;

	std::string outputType;
	std::string lastError;

	std::string lastRecordingPath;

	OBSSignal startRecording;
	OBSSignal stopRecording;
	OBSSignal startReplayBuffer;
	OBSSignal stopReplayBuffer;
	OBSSignal startStreaming;
	OBSSignal stopStreaming;
	OBSSignal streamDelayStarting;
	OBSSignal streamStopping;
	OBSSignal recordStopping;
	OBSSignal replayBufferStopping;
	OBSSignal replayBufferSaved;

	inline BasicOutputHandler(MainWindow *main_);

	virtual ~BasicOutputHandler(){};

	virtual bool SetupStreaming(obs_service_t *service) = 0;
	virtual bool StartStreaming(obs_service_t *service) = 0;
	virtual bool StartRecording() = 0;
	virtual bool StartReplayBuffer() { return false; }
	virtual void StopStreaming(bool force = false) = 0;
	virtual void StopRecording(bool force = false) = 0;
	virtual void StopReplayBuffer(bool force = false) { (void)force; }
	virtual bool StreamingActive() const = 0;
	virtual bool RecordingActive() const = 0;
	virtual bool ReplayBufferActive() const { return false; }

	virtual void Update() = 0;
	virtual void SetupOutputs() = 0;

	inline bool Active() const
	{
		return streamingActive || recordingActive || delayActive ||
		       replayBufferActive;
	}

protected:
	bool SetupAutoRemux(const char *&ext);
	std::string GetRecordingFilename(const char *path, const char *ext,
					 bool noSpace, bool overwrite,
					 const char *format, bool ffmpeg);
};

BasicOutputHandler *CreateSimpleOutputHandler(MainWindow *main);
BasicOutputHandler *CreateAdvancedOutputHandler(MainWindow *main);
