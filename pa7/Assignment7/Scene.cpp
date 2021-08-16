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
	Intersection inter_p = intersect(ray);
	if (!inter_p.happened)
	{
		return Vector3f(0, 0, 0);
	}

	const Vector3f& p = inter_p.coords;
	const Vector3f& N = normalize(inter_p.normal);
	Vector3f wo = normalize(-ray.direction);

	Intersection inter_light;
	float pdf_light;
	sampleLight(inter_light, pdf_light);

	const Vector3f& x = inter_light.coords;
	const Vector3f& NN = normalize(inter_light.normal);
	const Vector3f& emit = inter_light.emit;

	Vector3f px = x - p;
	Vector3f ws = normalize(px);

	float sqr_px = px.x*px.x + px.y*px.y + px.z*px.z;
	float sqrt_px = std::sqrt(sqr_px);

	Intersection inter_px = intersect(Ray(p, ws));

	Vector3f L_dir = inter_p.m->getEmission();
	if (inter_px.happened && std::abs(inter_px.distance-sqrt_px) <= EPSILON /*&& sqr_px > EPSILON && pdf_light > EPSILON*/)
	{
		L_dir += emit * inter_p.m->eval(wo, ws, N) * dotProduct(ws, N) * dotProduct(-ws, NN) / sqr_px / pdf_light;
	}

	Vector3f L_indir(0, 0, 0);
	float R = get_random_float();
	if (R < RussianRoulette)
	{
		Vector3f wi = normalize(inter_p.m->sample(wo, N));
		Intersection inter_q = intersect(Ray(p, wi));
		
		if (inter_q.happened && !inter_q.obj->hasEmit())
		{
			float pdf = inter_p.m->pdf(wo, wi, N);
			if (pdf > EPSILON)
			{
				L_indir = castRay(Ray(inter_p.coords, wi), ++depth) * inter_p.m->eval(wo, wi, N) * dotProduct(wi, N) / pdf / RussianRoulette;
			}
		}
	}

	return L_dir + L_indir;
}