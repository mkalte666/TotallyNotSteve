#include <stdio.h>
#include <AL/al.h>
#include <AL/alc.h>

#include <pocketsphinx.h>

#define AL_MIC_FREQ 16000
#define AL_MIC_CAP 2048

int main(int argc, char** argv)
{
    /* init ps */
    fprintf(stderr, "INIT PS\n");
    cmd_ln_t* psconfig = cmd_ln_init(NULL, ps_args(), TRUE,
        "-hmm", MODELDIR "/en-us/en-us",
        "-lm", MODELDIR "/en-us/en-us.lm.bin",
        "-dict", MODELDIR "/en-us/cmudict-en-us.dict",
        NULL);
    if (psconfig == NULL) {
        fprintf(stderr, "Cannot create config for PS\n");
        return -1;
    }
    
    ps_decoder_t* ps = ps_init(psconfig);
    if (ps == NULL) {
        fprintf(stderr, "Cannot create PS decoder\n");
        return -1;
    }

    fprintf(stderr, "INIT AL\n");
    /* init openal and create microphone */
    ALCdevice* aldevice = alcCaptureOpenDevice(NULL,AL_MIC_FREQ,AL_FORMAT_MONO16,AL_MIC_FREQ/2);
    if (aldevice == NULL) {
        fprintf(stderr, "Cannot open AL device");
        return -1;
    }
    alcCaptureStart(aldevice);

    /* conna capture some wods */
    ALCint samplesIn = 0;
    short buff[AL_MIC_FREQ*2];
    int ignoreCounter = 20;
    uint8 utt_started;
    uint8 in_speech;
    utt_started = FALSE;
    ps_start_utt(ps);
    const char* hyp;
    for (;;) {
        /* poll some data */
        alcGetIntegerv(aldevice,ALC_CAPTURE_SAMPLES,1,&samplesIn);
        if(samplesIn>AL_MIC_CAP) {
            alcCaptureSamples(aldevice,buff,AL_MIC_CAP);
            /* actual voice processing */
            ps_process_raw(ps, buff, AL_MIC_CAP, FALSE, FALSE);
            in_speech = ps_get_in_speech(ps);
            if (in_speech && !utt_started) {
                utt_started = TRUE;
                fprintf(stderr, "Hearing something i guess...\n");
            }
            if (!in_speech && utt_started) {
                fprintf(stderr, "Processing it now...\n");
                ps_end_utt(ps);
                hyp = ps_get_hyp(ps,NULL);
                if (hyp != NULL) {
                    fprintf(stderr, "Here we go!\n\n");
                    fprintf(stdout,"%s\n",hyp);
                    fflush(stdout);
                    fprintf(stderr,"\n\nTo the next round!\n");
                }
                ps_start_utt(ps);
                utt_started = FALSE;
            }

        }

    }
    return 0;
}
