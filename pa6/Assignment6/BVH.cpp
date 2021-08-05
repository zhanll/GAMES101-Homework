#include <algorithm>
#include <cassert>
#include "BVH.hpp"

const int BUCKET_NUM = 32;

BVHAccel::BVHAccel(std::vector<Object*> p, int maxPrimsInNode,
                   SplitMethod splitMethod)
    : maxPrimsInNode(std::min(255, maxPrimsInNode)), splitMethod(splitMethod),
      primitives(std::move(p))
{
    time_t start, stop;
    time(&start);
    if (primitives.empty())
        return;

    root = recursiveBuild(primitives);

    time(&stop);
    double diff = difftime(stop, start);
    int hrs = (int)diff / 3600;
    int mins = ((int)diff / 60) - (hrs * 60);
    int secs = (int)diff - (hrs * 3600) - (mins * 60);

    printf(
        "\rBVH Generation complete: \nTime Taken: %i hrs, %i mins, %i secs\n\n",
        hrs, mins, secs);
}

BVHBuildNode* BVHAccel::recursiveBuild(std::vector<Object*> objects)
{
    BVHBuildNode* node = new BVHBuildNode();

    // Compute bounds of all primitives in BVH node
    Bounds3 bounds;
    for (int i = 0; i < objects.size(); ++i)
        bounds = Union(bounds, objects[i]->getBounds());
    if (objects.size() == 1) {
        // Create leaf _BVHBuildNode_
        node->bounds = objects[0]->getBounds();
        node->object = objects[0];
        node->left = nullptr;
        node->right = nullptr;
        return node;
    }
    else if (objects.size() == 2) {
        node->left = recursiveBuild(std::vector{objects[0]});
        node->right = recursiveBuild(std::vector{objects[1]});

        node->bounds = Union(node->left->bounds, node->right->bounds);
        return node;
    }
    else {
        Bounds3 centroidBounds;
        for (int i = 0; i < objects.size(); ++i)
            centroidBounds =
                Union(centroidBounds, objects[i]->getBounds().Centroid());
        int dim = centroidBounds.maxExtent();
        switch (dim) {
        case 0:
            std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
                return f1->getBounds().Centroid().x <
                       f2->getBounds().Centroid().x;
            });
            break;
        case 1:
            std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
                return f1->getBounds().Centroid().y <
                       f2->getBounds().Centroid().y;
            });
            break;
        case 2:
            std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
                return f1->getBounds().Centroid().z <
                       f2->getBounds().Centroid().z;
            });
            break;
        }

        if (splitMethod == SplitMethod::NAIVE)
        {
			auto beginning = objects.begin();
			auto middling = objects.begin() + (objects.size() / 2);
			auto ending = objects.end();

			auto leftshapes = std::vector<Object*>(beginning, middling);
			auto rightshapes = std::vector<Object*>(middling, ending);

			assert(objects.size() == (leftshapes.size() + rightshapes.size()));

			node->left = recursiveBuild(leftshapes);
			node->right = recursiveBuild(rightshapes);
        }
        else if (splitMethod == SplitMethod::SAH)
        {
            double minSAH = DBL_MAX;
            std::vector<Object*> minLeft;
            std::vector<Object*> minRight;

            double p0 = centroidBounds.LongSideMin();
            double seg = centroidBounds.LongSideLength() / BUCKET_NUM;

			for (int i = 1; i <= BUCKET_NUM; ++i)
			{
                std::vector<Object*> outLeft;
                std::vector<Object*> outRight;
                splitObjects(objects, dim, p0+seg*i, outLeft, outRight);

                double sah = costSAH(centroidBounds, outLeft, outRight);
                if (sah < minSAH)
                {
                    minSAH = sah;
                    minLeft = outLeft;
                    minRight = outRight;
                }
			}

            node->left = recursiveBuild(minLeft);
            node->right = recursiveBuild(minRight);
        }

        node->bounds = Union(node->left->bounds, node->right->bounds);
    }

    return node;
}

double BVHAccel::costSAH(const Bounds3& wholeBound, const std::vector<Object*>& leftObjects, const std::vector<Object*>& rightObjects)
{
	Bounds3 leftBound;
	for (int i = 0; i < leftObjects.size(); ++i)
        leftBound = Union(leftBound, leftObjects[i]->getBounds());

	Bounds3 rightBound;
	for (int i = 0; i < rightObjects.size(); ++i)
        rightBound = Union(rightBound, rightObjects[i]->getBounds());

    double Sn = wholeBound.SurfaceArea();
    double Sl = leftBound.SurfaceArea();
    double Sr = rightBound.SurfaceArea();

    double Pl = Sl / Sn;
    double Pr = Sr / Sn;

    int Nl = leftObjects.size();
    int Nr = rightObjects.size();

    return (Sl * Nl + Sr * Nr) / Sn;
}

void BVHAccel::splitObjects(const std::vector<Object*> objects, int dim, double border, std::vector<Object*>& outLeft, std::vector<Object*>& outRight)
{
	switch (dim)
	{
	    case 0:
	    {
		    for (Object* obj : objects)
		    {
			    if (obj->getBounds().Centroid().x <= border)
			    {
				    outLeft.push_back(obj);
			    }
			    else
			    {
				    outRight.push_back(obj);
			    }
		    }
	    }
	    break;
	    case 1:
	    {
		    for (Object* obj : objects)
		    {
			    if (obj->getBounds().Centroid().y <= border)
			    {
				    outLeft.push_back(obj);
			    }
			    else
			    {
				    outRight.push_back(obj);
			    }
		    }
	    }
	    break;
	    case 2:
	    {
		    for (Object* obj : objects)
		    {
			    if (obj->getBounds().Centroid().z <= border)
			    {
				    outLeft.push_back(obj);
			    }
			    else
			    {
				    outRight.push_back(obj);
			    }
		    }
	    }
	    break;
	}
}

Intersection BVHAccel::Intersect(const Ray& ray) const
{
    Intersection isect;
    if (!root)
        return isect;
    isect = BVHAccel::getIntersection(root, ray);
    return isect;
}

Intersection BVHAccel::getIntersection(BVHBuildNode* node, const Ray& ray) const
{
    // TODO Traverse the BVH to find intersection
    std::array<int, 3> dirIsNeg={ray.direction.x>0, ray.direction.y>0, ray.direction.z>0};
    if (node->bounds.IntersectP(ray, ray.direction_inv, dirIsNeg))
    {
        if (!node->left && !node->right)
        {
            return node->object->getIntersection(ray);
        }
        else
        {
            Intersection InterLeft, InterRight;
            if (node->left)
            {
                InterLeft = getIntersection(node->left, ray);
            }
            if (node->right)
            {
                InterRight = getIntersection(node->right, ray);
            }

            if (InterLeft.happened && InterRight.happened)
            {
                return InterLeft.distance>InterRight.distance ? InterRight : InterLeft;
            }
            else if (InterLeft.happened)
            {
                return InterLeft;
            }
            else
            {
                return InterRight;
            }
            
            
        }
        
    }
    
    return Intersection();
}