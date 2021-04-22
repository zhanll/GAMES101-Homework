#include <iostream>
#include <vector>

#include "CGL/vector2D.h"

#include "mass.h"
#include "rope.h"
#include "spring.h"

namespace CGL {

    Rope::Rope(Vector2D start, Vector2D end, int num_nodes, float node_mass, float k, vector<int> pinned_nodes)
    {
        // TODO (Part 1): Create a rope starting at `start`, ending at `end`, and containing `num_nodes` nodes.
        Vector2D start_to_end = end - start;
        float length_total = start_to_end.norm();
        Vector2D unit_start_to_end = start_to_end / length_total;
        float length_section = length_total / (num_nodes - 1);
        for (size_t i = 0; i < num_nodes; i++)
        {
            Vector2D point = start + unit_start_to_end * length_section * i;
            Mass* m = new Mass(point, node_mass, false);
            masses.push_back(m);

            if (i > 0 && i < num_nodes)
            {
                Spring* s = new Spring(masses[i-1], m, k);
                springs.push_back(s);
            }
            
        }
        

        // Comment-in this part when you implement the constructor
        for (auto &i : pinned_nodes) {
            masses[i]->pinned = true;
        }
    }

    void Rope::simulateEuler(float delta_t, Vector2D gravity)
    {
        for (auto &s : springs)
        {
            // TODO (Part 2): Use Hooke's law to calculate the force on a node
            Vector2D a_to_b = s->m2->position - s->m1->position;
            float length_ab = a_to_b.norm();
            Vector2D f = s->k * a_to_b * (length_ab - s->rest_length) / length_ab;
            s->m1->forces += f;
            s->m2->forces -= f;
        }

        for (auto &m : masses)
        {
            if (!m->pinned)
            {
                // TODO (Part 2): Add the force due to gravity, then compute the new velocity and position
                Vector2D f = m->forces + gravity;

                // TODO (Part 2): Add global damping
                //const float k_d = 6.5f;     // explicit euler
                const float k_d = 0.01f;    // semi implicit euler
                f -= k_d * m->velocity;

                Vector2D a = f / m->mass;
                m->velocity += a * delta_t; // semi implicit euler
                m->position += m->velocity * delta_t;
                //m->velocity += a * delta_t; // explicit euler
            }

            // Reset all forces on each mass
            m->forces = Vector2D(0, 0);
        }
    }

    void Rope::simulateVerlet(float delta_t, Vector2D gravity)
    {
        for (auto &s : springs)
        {
            // TODO (Part 3): Simulate one timestep of the rope using explicit Verlet （solving constraints)
            Vector2D a_to_b = s->m2->position - s->m1->position;
            float length_ab = a_to_b.norm();
            Vector2D unit_ab = a_to_b / length_ab;
            Vector2D f = s->k * a_to_b * (length_ab - s->rest_length) / length_ab;
            s->m1->forces += f;
            s->m2->forces -= f;
        }

        for (auto &m : masses)
        {
            if (!m->pinned)
            {
                Vector2D temp_position = m->position;
                // TODO (Part 3.1): Set the new position of the rope mass
                Vector2D f = m->forces + gravity;
                Vector2D a = f / m->mass;
                Vector2D x = m->position - m->last_position;
                
                // TODO (Part 4): Add global Verlet damping
                const float damping_factor = 0.0001f;
                m->position += (1 - damping_factor) * x + a * delta_t * delta_t;

                m->last_position = temp_position;
            }

            // Reset all forces on each mass
            m->forces = Vector2D(0, 0);
        }

        for (auto &s : springs)
        {
            Vector2D a_to_b = s->m2->position - s->m1->position;
            float length_ab = a_to_b.norm();
            Vector2D unit_ab = a_to_b / length_ab;

            float c_ab = length_ab - s->rest_length;
            float m_ar = 1.0f / s->m1->mass;
            float m_br = 1.0f / s->m2->mass;

            if (!s->m1->pinned)
            {
                s->m1->position += m_ar * c_ab / (m_ar + m_br) * unit_ab;
            }

            if (!s->m2->pinned)
            {
                s->m2->position += -m_br * c_ab / (m_ar + m_br) * unit_ab;
            }
        }
    }
}
