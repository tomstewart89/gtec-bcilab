%
% A matlab example of sending data to OpenViBE via LabStreamingLayer. You need to have the LSL matlab stuff recursively on Matlab's Path.
%
% At the time of writing this, OpenViBE only supports numeric markers.
%
nChannels = 8;
samplingFreq = 100;

disp('Loading LSL library...');
lib = lsl_loadlib();

% make a new stream outlet
disp('Creating a new signal stream ...');
info = lsl_streaminfo(lib,'ACMEAmp','EEG',nChannels,samplingFreq,'cf_float32','abcdefgh');

disp('Opening a signal outlet...');
outlet = lsl_outlet(info);

disp('Creating a new marker stream ...');
markerInfo = lsl_streaminfo(lib,'ACMEMarkers','Markers',1,0,'cf_int32','myuniquesourceid23443');

disp('Opening a marker outlet...');
markerOutlet = lsl_outlet(markerInfo);

disp('Sending data...');
t=0;
value = -1;
while true
	
	% If current time is even, sample is positive. When sample value changes to positive, send a marker.
    sendMarker = 0;
	if(mod(floor(t),2)==0)
	  if(value~=1)
	     sendMarker=1;
	  end
	  value = 1;
	else
	  value = -1;
	end

	% Push a sample
    outlet.push_sample(value*(1:nChannels),t);	
	
	% Push a marker?
	if(sendMarker>0)
	  disp('push marker')
	  markerOutlet.push_sample(777,t);
	end
	
	% step time, wait
	t=t+(1/samplingFreq);
    pause(1/samplingFreq);
end

