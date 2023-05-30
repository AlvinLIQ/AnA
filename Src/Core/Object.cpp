#include "Headers/Object.hpp"
#include "RenderSystem/Headers/RenderSystem.hpp"

using namespace AnA;

Object::id_t currentId = 0;

Object::Object()
{
    id = currentId++;
    RenderSystems::RenderSystem::GetCurrent()->CreateDescriptorSets(descriptorSets);
}

Object::~Object()
{
    if (Model != nullptr)
    {
        Model.reset();
        //Model = nullptr;
    }
}

Texture* Object::GetTexture()
{
    return texture.get();
}

void Object::SetTexture(std::shared_ptr<Texture> newTexture)
{
    if (texture.get() == newTexture.get())
        return;
    
    texture = newTexture;
    for (size_t i = 0; i < descriptorSets.size(); i++)
    {
        if (texture != nullptr)
        {
            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = texture->GetImageView();
            imageInfo.sampler = texture->GetSampler();

            VkWriteDescriptorSet descriptorWrite{};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = descriptorSets[i];
            descriptorWrite.dstBinding = 1;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pImageInfo = &imageInfo;

            vkUpdateDescriptorSets(texture->GetDevice().GetLogicalDevice(), 1,
                &descriptorWrite, 0, nullptr);
        }
    }
    //RenderSystems::RenderSystem::GetCurrent()->CreateDescriptorSets(descriptorSets, texture.get());
}

void Object::PrepareDraw()
{

}