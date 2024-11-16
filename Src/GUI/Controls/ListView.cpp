#include "Headers/ListView.hpp"

using namespace AnA;
using namespace Controls;

ListView::ListView()
{

}

void ListView::PrepareDraw(Shape* shapeBuffer, std::vector<VkDescriptorImageInfo>& imageInfos, uint32_t& shapeCount)
{
    StackPanel::PrepareDraw(shapeBuffer, imageInfos, shapeCount);
}
