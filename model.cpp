#include "model.h"
#include <Box2D/Box2D.h>
#include <stdio.h>
#include <QDebug>

Model::Model(QObject *parent)
    : QObject{parent}, world(b2World(b2Vec2(0.0f, 100.0f)))
{
    // Timer setup
    timer = new QTimer(this);
    connect (timer,
            &QTimer::timeout,
            this,
            &Model::UpdateView);

    // Make a circle body
//    b2BodyDef circleDef;
//    circleDef.type = b2_dynamicBody;
//    circleDef.position.Set(50.0f, 20.0f);
//    b2Body* circle = world.CreateBody(&circleDef);
//    b2CircleShape circleShape;
//    circleShape.m_radius = 2.0f;
//    circle->CreateFixture(&circleShape, 0.0f);

//    b2BodyDef floorDef;
//    floorDef.type = b2_staticBody;
//    floorDef.position.Set(0.0f, 0.0f);
//    b2Body* floor = world.CreateBody(&floorDef);
//    b2PolygonShape floorShape;
//    floorShape.SetAsBox(50.0f, 10.0f);
//    floor->CreateFixture(&floorShape, 0.0f);

//    bodies.push_back(circle);
//    bodies.push_back(floor);

//        // Define the gravity vector.
//        b2Vec2 gravity(0.0f, -10.0f);

//        // Construct a world object, which will hold and simulate the rigid bodies.
//        b2World world(gravity);

        // Define the ground body.
        b2BodyDef groundBodyDef;

        // Set up the ground position. 90 seems to match the gameview's ground
        groundBodyDef.position.Set(0.0f, 90.0f);

        // Call the body factory which allocates memory for the ground body
        // from a pool and creates the ground box shape (also from a pool).
        // The body is also added to the world.
        b2Body* groundBody = world.CreateBody(&groundBodyDef);
        // Define the ground box shape.
        b2PolygonShape groundBox;
        // The extents are the half-widths of the box.
        groundBox.SetAsBox(500.0f, 10.0f);
        // Add the ground fixture to the ground body.
        groundBody->CreateFixture(&groundBox, 0.0f);

        // Define the dynamic body. We set its position and call the body factory.
        b2BodyDef bodyDef;
        bodyDef.type = b2_dynamicBody;

        // Set up the initial position of the body
        bodyDef.position.Set(0.0f, 0.0f);

        b2Body* body = world.CreateBody(&bodyDef);
        // Define another box shape for our dynamic body.
        b2PolygonShape dynamicBox;
        dynamicBox.SetAsBox(1.0f, 1.0f);
        // Define the dynamic body fixture.
        b2FixtureDef fixtureDef;
        fixtureDef.shape = &dynamicBox;

        fixtureDef.density = 1.0f;
        fixtureDef.friction = 0.3f;
        // Restitution sets up bounciness.
        fixtureDef.restitution = 1.0f;

        // Add the shape to the body.
        body->CreateFixture(&fixtureDef);

        bodies.push_back(body);
        bodies.push_back(groundBody);

        // Create a repository for all potential Atoms in the game.
        for (unsigned int i = 0; i < 10; i++)
        {
            elementList.push_back(new Atom (nullptr, i-1));
        }
}

void Model::BeginGame()
{
//   // This is our little game loop.
//    for (int32 i = 0; i < 60; ++i)
//    {
//        // Instruct the world to perform a single step of simulation.
//        // It is generally best to keep the time step and iterations fixed.
//        world.Step(timeStep, velocityIterations, positionIterations);
//        b2Vec2 position = bodies[0]->GetPosition();
//        qDebug() << "m position: " << position.x << " " << position.y;
//        emit UpdateWorld();
//    }
    timer->start(1000/60);
}

void Model::UpdateView()
{
    world.Step(timeStep, velocityIterations, positionIterations);
    b2Vec2 position = bodies[0]->GetPosition();
    qDebug() << "position: " << position.x << " " << position.y;
    emit UpdateWorld();
}

void Model::SendBodiesTemp()
{
    emit SendBodies(bodies);
    BeginGame();
}

// Box2D code from lab14
//int main(int argc, char** argv)
//{
//    B2_NOT_USED(argc);
//    B2_NOT_USED(argv);

//    // Define the gravity vector.
//    b2Vec2 gravity(0.0f, -10.0f);

//    // Construct a world object, which will hold and simulate the rigid bodies.
//    b2World world(gravity);

//    // Define the ground body.
//    b2BodyDef groundBodyDef;
//    groundBodyDef.position.Set(0.0f, -10.0f);

//    // Call the body factory which allocates memory for the ground body
//    // from a pool and creates the ground box shape (also from a pool).
//    // The body is also added to the world.
//    b2Body* groundBody = world.CreateBody(&groundBodyDef);

//    // Define the ground box shape.
//    b2PolygonShape groundBox;

//    // The extents are the half-widths of the box.
//    groundBox.SetAsBox(50.0f, 10.0f);

//    // Add the ground fixture to the ground body.
//    groundBody->CreateFixture(&groundBox, 0.0f);

//    // Define the dynamic body. We set its position and call the body factory.
//    b2BodyDef bodyDef;
//    bodyDef.type = b2_dynamicBody;
//    bodyDef.position.Set(0.0f, 4.0f);
//    b2Body* body = world.CreateBody(&bodyDef);

//    // Define another box shape for our dynamic body.
//    b2PolygonShape dynamicBox;
//    dynamicBox.SetAsBox(1.0f, 1.0f);

//    // Define the dynamic body fixture.
//    b2FixtureDef fixtureDef;
//    fixtureDef.shape = &dynamicBox;

//    // Set the box density to be non-zero, so it will be dynamic.
//    fixtureDef.density = 1.0f;

//    // Override the default friction.
//    fixtureDef.friction = 0.3f;

//    // Add the shape to the body.
//    body->CreateFixture(&fixtureDef);

//    // Prepare for simulation. Typically we use a time step of 1/60 of a
//    // second (60Hz) and 10 iterations. This provides a high quality simulation
//    // in most game scenarios.
//    float32 timeStep = 1.0f / 60.0f;
//    int32 velocityIterations = 6;
//    int32 positionIterations = 2;

//    // This is our little game loop.
//    for (int32 i = 0; i < 60; ++i)
//    {
//        // Instruct the world to perform a single step of simulation.
//        // It is generally best to keep the time step and iterations fixed.
//        world.Step(timeStep, velocityIterations, positionIterations);

//        // Now print the position and angle of the body.
//        b2Vec2 position = body->GetPosition();
//        float32 angle = body->GetAngle();

//        printf("%4.2f %4.2f %4.2f\n", position.x, position.y, angle);
//    }

//    // When the world destructor is called, all bodies and joints are freed. This can
//    // create orphaned pointers, so be careful about your world management.

//    return 0;
//}
