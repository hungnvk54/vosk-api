// Copyright 2019 Alpha Cephei Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "base/kaldi-common.h"
#include "util/common-utils.h"
#include "fstext/fstext-lib.h"
#include "fstext/fstext-utils.h"
#include "decoder/lattice-faster-decoder.h"
#include "feat/feature-mfcc.h"
#include "lat/kaldi-lattice.h"
#include "lat/word-align-lattice.h"
#include "nnet3/am-nnet-simple.h"
#include "nnet3/nnet-am-decodable-simple.h"
#include "nnet3/nnet-utils.h"

#include "model.h"
#include "spk_model.h"

using namespace kaldi;


class ResultData {
    
    public:
        ResultData (const ResultData &from){
            word_ = from.word_;
            start_ = from.start_;
            end_ = from.end_;
            conf_ = from.conf_;
        }
    
    public:
        ResultData(std::string word, float start, float end, float conf){
            word_ = word;
            start_ = start;
            end_ = end;
            conf_ = conf;
        }
    
    public:
        std::string word_;
        float start_;
        float end_;
        float conf_;
};

class KaldiRecognizer {
    public:
        KaldiRecognizer(Model *model, float sample_frequency, float time_offset = 0);
        KaldiRecognizer(Model *model, SpkModel *spk_model, float sample_frequency, float time_offset = 0);
        KaldiRecognizer(Model *model, float sample_frequency, char const *grammar, float time_offset = 0);
        ~KaldiRecognizer();
        bool AcceptWaveform(const char *data, int len);
        bool AcceptWaveform(const short *sdata, int len);
        bool AcceptWaveform(const float *fdata, int len);
        const char* Result();
        const char* FinalResult();
        const char* PartialResult();

    private:
        void BufferResult();
        void DecodeBeforeResetMemory();
    private:
        void InitRescoring();
        void CleanUp();
        void UpdateSilenceWeights();
        bool AcceptWaveform(Vector<BaseFloat> &wdata);
        void GetSpkVector(Vector<BaseFloat> &xvector);

        Model *model_;
        SingleUtteranceNnet3Decoder *decoder_;
        fst::LookaheadFst<fst::StdArc, int32> *decode_fst_;
        fst::StdVectorFst *g_fst_; // dynamically constructed grammar
        OnlineNnet2FeaturePipeline *feature_pipeline_;
        OnlineSilenceWeighting *silence_weighting_;

        SpkModel *spk_model_;
        OnlineBaseFeature *spk_feature_;

        fst::MapFst<fst::StdArc, kaldi::LatticeArc, fst::StdToLatticeMapper<kaldi::BaseFloat> > *lm_fst_;

        float sample_frequency_;
        int32 frame_offset_;
        int32 round_offset_;
        bool input_finalized_;
        string last_result_;
        float time_offset_;
        stringstream text_;
        std::vector<ResultData> buffer_data;
};
