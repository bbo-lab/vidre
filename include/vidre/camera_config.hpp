#include <string>
#include <pylon/PylonIncludes.h>

namespace vidre {
    // Is for now geared towards GenICam API. Might be better to abstract so that e.g. Aptina can have its own config communication implementation.
    class CameraConfig {
    public:
        void SetNodeMap(GenApi::INodeMap &nodeMap);
        GenApi::INodeMap &GetNodeMap();

        void SetBool(std::string parameter, bool value);
        void SetInt(std::string parameter, int64_t value);
        void SetDouble(std::string parameter, double value);
        bool GetBool(std::string parameter);
        int64_t GetInt(std::string parameter);
        double GetDouble(std::string parameter);
        //void SetExecute(std::string parameter);

        bool SaveConfiguration(std::string filepath);
        bool LoadConfiguration(std::string filepath);

        unsigned int GetFrameWidth();
        unsigned int GetFrameHeight();
        unsigned int GetFrameOffsetX();
        unsigned int GetFrameOffsetY();
        unsigned int GetMaxNumBuffer();
        double GetFrameRate();
        void SetFrameWidth(unsigned int newFrameWidth);
        void SetFrameHeight(unsigned int newFrameHeight);
        void SetFrameOffsetX(unsigned int newFrameOffsetX);
        void SetFrameOffsetY(unsigned int newFrameOffsetY);
        void SetFrameRate(double newFrameRate);
    };
}