#include "/home/codeleaded/System/Static/Library/WindowEngine1.0.h"
#include "/home/codeleaded/System/Static/Library/Random.h"
#include "/home/codeleaded/System/Static/Container/Vector.h"
#include "/home/codeleaded/System/Static/Library/TransformedView.h"
#include "/home/codeleaded/System/Static/Container/SQuadTree.h"

/*
	Vector -> einfach, default Resize: 5
	Vector -> Sehr schnell bei großen datenmengen
*/

typedef struct AObject {
	Vec2 p;
	Vec2 d;
	Vec2 v;
	Pixel c;
} AObject;

TransformedView tv;
Vector Objects;
SQuadTree TreeObjects;
float Area = 100000.0f;
size_t count = 1000000;
char UseTree = False;

void Item_UpdateDraw(AObject* obj){
	const Vec2 p = TransformedView_WorldScreenPos(&tv,obj->p);
	const Vec2 d = TransformedView_WorldScreenLength(&tv,obj->d);
	RenderRect(p.x,p.y,d.x,d.y,obj->c);
}

void Setup(AlxWindow* w){
	tv = TransformedView_New((Vec2){ GetWidth(),GetHeight() });
	TransformedView_Zoom(&tv,(Vec2){ 0.05f,0.05f });

    Objects = Vector_New(sizeof(AObject));
	TreeObjects = SQuadTree_New((Rect){ { 0.0f,0.0f },{ Area,Area }},10);

	for(int i = 0;i<count;i++){
		AObject Obj = {
			{ Random_f64_MinMax(0.0f,Area),Random_f64_MinMax(0.0f,Area) },
			{ Random_f64_MinMax(10.0f,100.0f),Random_f64_MinMax(10.0f,100.0f) },
			{ 0.0f,0.0f },
			Pixel_toRGBA(Random_f64_New(),Random_f64_New(),Random_f64_New(),1.0f)
		};
		Vector_Push(&Objects,(AObject[]){ Obj });
		SQuadTree_Add(&TreeObjects,(AObject[]){ Obj },sizeof(AObject),(Rect){ Obj.p,Obj.d });
	}
}

void Update(AlxWindow* w){
    TransformedView_HandlePanZoom(&tv,window.Strokes,(Vec2){ GetMouse().x,GetMouse().y });
	Rect Screen = TransformedView_ScreenWorldRect(&tv,(Rect){ 0.0f,0.0f,GetWidth(),GetHeight() });
	int ObjectCount = 0;

	if(Stroke(ALX_KEY_TAB).PRESSED){
		UseTree = !UseTree;
	}

	Clear(BLACK);
	
	if(UseTree){
		const Timepoint start = Time_Nano();
		
		Vector visible = SQuadTree_Search(&TreeObjects,Screen);
		const int ObjectCount = visible.size;
		for(int i = 0;i<visible.size;i++){
    	    SQuadTree_Item* qti = *(SQuadTree_Item**)Vector_Get(&visible,i);
			Item_UpdateDraw((AObject*)qti->data);
    	}
		Vector_Free(&visible);

		const FDuration ElapsedTime = Time_Elapsed(start,Time_Nano());
		CStr_RenderAlxFontf(WINDOW_STD_ARGS,GetAlxFont(),0.0f,0.0f,WHITE,"QuadTree (%d/%d) in %f",ObjectCount,count,ElapsedTime);
	}else{
		const Timepoint start = Time_Nano();

		for(int i = 0;i<Objects.size;i++){
    	    const AObject* obj = (AObject*)Vector_Get(&Objects,i);
			
			if(Overlap_Rect_Rect(Screen,(Rect){ obj->p,obj->d })){
				Vec2 p = TransformedView_WorldScreenPos(&tv,obj->p);
				Vec2 d = TransformedView_WorldScreenLength(&tv,obj->d);
				RenderRect(p.x,p.y,d.x,d.y,obj->c);
				ObjectCount++;
			}
    	}

		const FDuration ElapsedTime = Time_Elapsed(start,Time_Nano());
		CStr_RenderAlxFontf(WINDOW_STD_ARGS,GetAlxFont(),0.0f,0.0f,WHITE,"Linear (%d/%d) in %f",ObjectCount,count,ElapsedTime);
	}

	CStr_RenderAlxFontf(WINDOW_STD_ARGS,GetAlxFont(),0.0f,GetAlxFont()->CharSizeY,WHITE,"O: %f,%f | Z: %f,%f",tv.Offset.x,tv.Offset.y,tv.Scale.x,tv.Scale.y);
}

void Delete(AlxWindow* w){
    Vector_Free(&Objects);
	SQuadTree_Free(&TreeObjects);
}

int main(){
    if(Create("Vector vs Quad Tree (Static)",2500,1300,1,1,Setup,Update,Delete))
        Start();
    return 0;
}