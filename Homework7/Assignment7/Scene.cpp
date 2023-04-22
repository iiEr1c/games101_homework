//
// Created by Göksu Güvendiren on 2019-05-14.
//

#include "Scene.hpp"


void Scene::buildBVH() {
    printf(" - Generating BVH...\n\n");
    this->bvh = new BVHAccel(objects, 1, BVHAccel::SplitMethod::NAIVE);
}

Intersection Scene::intersect(const Ray &ray) const
{
    return this->bvh->Intersect(ray);
}

void Scene::sampleLight(Intersection &pos, float &pdf) const
{
    float emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
        }
    }
    float p = get_random_float() * emit_area_sum;
    emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
            if (p <= emit_area_sum){
                objects[k]->Sample(pos, pdf);
                break;
            }
        }
    }
}

bool Scene::trace(
        const Ray &ray,
        const std::vector<Object*> &objects,
        float &tNear, uint32_t &index, Object **hitObject)
{
    *hitObject = nullptr;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        float tNearK = kInfinity;
        uint32_t indexK;
        Vector2f uvK;
        if (objects[k]->intersect(ray, tNearK, indexK) && tNearK < tNear) {
            *hitObject = objects[k];
            tNear = tNearK;
            index = indexK;
        }
    }


    return (*hitObject != nullptr);
}

// Implementation of Path Tracing
Vector3f Scene::castRay(const Ray &ray, int depth) const
{
    // TO DO Implement Path Tracing Algorithm here
    Vector3f result{0.0f, 0.0f, 0.0f};
    Intersection intersection = intersect(ray);
    if (intersection.happened) {
        result = shade(intersection, -ray.direction);
    }

    return result;
}


// 其实这里是从像素点开始打到的第一个object上开始计算的.
// 这里的wo和ppt上的一致. 注意
// assume wo is normalized.
Vector3f Scene::shade(const Intersection& p, const Vector3f& wo) const {
    // 如果是光源那么直接返回
    if(p.m->hasEmission()) {
        return p.m->getEmission();
    }

    constexpr float epsilon = 0.0005f;

    Vector3f L_dir{0.0f, 0.0f, 0.0f}; // light_direct
    Vector3f L_indir{0.0f, 0.0f, 0.0f}; // light_indirect


    // 对光源进行采样
    Intersection lightInter;
    float pdf;
    sampleLight(lightInter, pdf);

    // 判断当前位置能否被光线直射
    auto p_to_light(lightInter.coords - p.coords);
    auto dir_p_to_light = normalize(p_to_light);
    Ray ray_p_to_light(p.coords, dir_p_to_light);
    // p沿着dir_p_to_light方向最近的交点
    auto inter_nearest_p = Scene::intersect(ray_p_to_light);
    if (std::abs(p_to_light.norm() - inter_nearest_p.distance) < epsilon) {
        // 认为光线能够直射p
        // 提供wi/wo/和对应平面的法线
        auto f_r = p.m->eval(dir_p_to_light, wo, p.normal);
        L_dir = lightInter.emit * f_r *
                std::max(0.0f, dotProduct(dir_p_to_light, p.normal)) *
                std::max(0.0f, dotProduct(-dir_p_to_light, lightInter.normal)) /
                (dotProduct(p_to_light, p_to_light)) / pdf;
    }

    if(get_random_float() < RussianRoulette) {
        // 注意这里wo的方向.
        auto wi = p.m->sample(wo, normalize(p.normal)).normalized();
        auto randomRay = Ray(p.coords, wi);
        auto pdf = p.m->pdf(wi, wo, p.normal);
        // 因为pdf也可能非常小
        if (pdf > epsilon) {
            auto nextObj = Scene::intersect(randomRay);
            // 非光源, 因为光源在上边已经计算过了
            if(nextObj.happened && !nextObj.m->hasEmission()) {
                auto f_r = p.m->eval(wi, wo, p.normal);
                L_indir = shade(nextObj, -wi) * f_r *
                        std::max(0.0f, dotProduct(wi, p.normal)) / pdf;
            }
        }
    }

    return p.m->getEmission() + L_dir + L_indir;
}