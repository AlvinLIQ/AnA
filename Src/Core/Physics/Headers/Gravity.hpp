#pragma once

#define DEFAULT_G 10

namespace AnA
{
    namespace Physics
    {
        class Gravity
        {
        public:
            Gravity();

            void Drop(float sTime);
            void StopDropping();
            
            float GetDropDistance(float curTime);
            
            const float G = DEFAULT_G;
        private:
            float startTime;
        };
    }
}