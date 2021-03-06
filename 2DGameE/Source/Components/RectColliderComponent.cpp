#include "RectColliderComponent.h"
#include "TransformComponent.h"
#include "CameraManager.h"
#include "../game.h"
#include "../Map.h"
#include "../EngineMath.h"
#include "../CustomShapes.h"


RectColliderComponent::RectColliderComponent(std::string colliderTag) : ColliderComponentBase(colliderTag) 
{
	colliderType = COLLIDER_RECT;
}

// To delete
SDL_Rect inline RectColliderComponent::getRect()
{
	// uncoment if you don't set x and y by transform->Location + cam loc (And comment rest)
	//SDL_Rect newRect = transform->Rect;

	SDL_Rect newRect;
	newRect.w = transform->Rect.w;
	newRect.h = transform->Rect.h;

	newRect.x = transform->Location.X + entity->getManager().game->camera->getComponent<CameraManagerComponent>().getCameraPosition().X;
	newRect.y = transform->Location.Y + entity->getManager().game->camera->getComponent<CameraManagerComponent>().getCameraPosition().Y;

	return newRect;
}

Vector2D<int> inline RectColliderComponent::getCenter()
{
	return EMath::GetRectCenter(getRect());
}

void RectColliderComponent::customUpdate()
{
	Vector2D<int> * vertices = EMath::RotateRect(getRect(), getCenter(), 0);

	vertex_TL = vertices[0];
	vertex_TR = vertices[1];
	vertex_BL = vertices[2];
	vertex_BR = vertices[3];
}

void RectColliderComponent::customRender()
{
#ifdef _DEBUG
	if (drawDebugCollider)
	{
		Vector2D<> camLoc = entity->getManager().game->camera->getComponent<CameraManagerComponent>().getCameraPosition();

		SDL_SetRenderDrawColor(Game::renderer, colliderColor.r, colliderColor.g, colliderColor.b, colliderColor.a);

		// Debug collider
		SDL_RenderDrawLine(Game::renderer, vertex_TL.X, vertex_TL.Y, vertex_TR.X, vertex_TR.Y);
		SDL_RenderDrawLine(Game::renderer, vertex_TR.X, vertex_TR.Y, vertex_BR.X, vertex_BR.Y);
		SDL_RenderDrawLine(Game::renderer, vertex_BR.X, vertex_BR.Y, vertex_BL.X, vertex_BL.Y);
		SDL_RenderDrawLine(Game::renderer, vertex_BL.X, vertex_BL.Y, vertex_TL.X, vertex_TL.Y);

		// Get center of collider
		Vector2D<int> center = getCenter();

		// Debug circle
		SDL_RenderDrawPoint(Game::renderer, center.X, center.Y);
		// Debug circle - Bigger
		Shapes::drawCircle(Game::renderer, center.X, center.Y, 2);
	}
#endif
}
