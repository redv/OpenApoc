
#include "game/debugtools/formpreview.h"
#include "framework/framework.h"

namespace OpenApoc
{

FormPreview::FormPreview() : Stage()
{
	displayform = nullptr;
	currentSelected = nullptr;
	currentSelectedControl = nullptr;
	glowindex = 0;

	previewselector.reset(new Form(nullptr));
	previewselector->Size = {200, 300};
	previewselector->Location = {2, 2};
	previewselector->BackgroundColour = {192, 192, 192, 255};

	Control *ch = new Control(previewselector.get());
	ch->Location = {2, 2};
	ch->Size = previewselector->Size - 4;
	ch->BackgroundColour = {80, 80, 80, 255};

	Control *c = new Control(ch);
	c->Location = {2, 2};
	c->Size = previewselector->Size - 4;
	c->BackgroundColour = {80, 80, 80, 255};

	Label *l = new Label(c, "Pick Form:", fw().gamecore->GetFont("SMALFONT"));
	l->Location = {0, 0};
	l->Size.x = c->Size.x;
	l->Size.y = fw().gamecore->GetFont("SMALFONT")->GetFontHeight();
	l->BackgroundColour = {80, 80, 80, 255};

	interactWithDisplay = new CheckBox(
	    c, fw().data->load_image(
	           "PCK:XCOM3/UFODATA/NEWBUT.PCK:XCOM3/UFODATA/NEWBUT.TAB:65:UI/menuopt.pal"),
	    fw().data->load_image(
	        "PCK:XCOM3/UFODATA/NEWBUT.PCK:XCOM3/UFODATA/NEWBUT.TAB:64:UI/menuopt.pal"));
	interactWithDisplay->Size = {19, 16};
	interactWithDisplay->Location.x = 0;
	interactWithDisplay->Location.y = c->Size.y - interactWithDisplay->Size.y;
	interactWithDisplay->BackgroundColour = {80, 80, 80, 255};
	interactWithDisplay->SetChecked(true);

	l = new Label(c, "Interact?", fw().gamecore->GetFont("SMALFONT"));
	l->Location.x = interactWithDisplay->Size.x + 2;
	l->Location.y = interactWithDisplay->Location.y;
	l->Size.x = c->Size.x - l->Location.x;
	l->Size.y = interactWithDisplay->Size.y;
	l->BackgroundColour = {80, 80, 80, 255};

	ListBox *lb = new ListBox(c);
	lb->Location.x = 0;
	lb->Location.y = fw().gamecore->GetFont("SMALFONT")->GetFontHeight();
	lb->Size.x = c->Size.x;
	lb->Size.y = interactWithDisplay->Location.y - lb->Location.y;
	lb->Name = "FORM_LIST";
	lb->ItemSize = lb->Location.y;
	lb->BackgroundColour = {24, 24, 24, 255};

	std::vector<UString> idlist = fw().gamecore->GetFormIDs();
	for (auto idx = idlist.begin(); idx != idlist.end(); idx++)
	{
		l = new Label(lb, (UString)*idx, fw().gamecore->GetFont("SMALFONT"));
		l->Name = l->GetText();
		l->BackgroundColour = {192, 80, 80, 0};
		// lb->AddItem( l );
	}

	propertyeditor.reset(new Form(nullptr));
	propertyeditor->Location = {2, 304};
	propertyeditor->Size.x = 200;
	propertyeditor->Size.y = fw().Display_GetHeight() - propertyeditor->Location.y - 2;
	propertyeditor->BackgroundColour = {192, 192, 192, 255};
}

FormPreview::~FormPreview() {}

void FormPreview::Begin() {}

void FormPreview::Pause() {}

void FormPreview::Resume() {}

void FormPreview::Finish() {}

void FormPreview::EventOccurred(Event *e)
{
	previewselector->EventOccured(e);
	if (propertyeditor != nullptr)
	{
		propertyeditor->EventOccured(e);
	}
	if (displayform != nullptr && interactWithDisplay->IsChecked())
	{
		displayform->EventOccured(e);
	}
	fw().gamecore->MouseCursor->EventOccured(e);

	if (e->Type == EVENT_KEY_DOWN)
	{
		if (e->Data.Keyboard.KeyCode == SDLK_ESCAPE)
		{
			stageCmd.cmd = StageCmd::Command::POP;
			return;
		}
	}

	if (e->Type == EVENT_FORM_INTERACTION && e->Data.Forms.EventFlag == FormEventType::MouseClick)
	{

		if (displayform != nullptr && e->Data.Forms.RaisedBy->GetForm() == displayform.get())
		{
			currentSelectedControl = e->Data.Forms.RaisedBy;
			ConfigureSelectedControlForm();
		}
		else
		{
			currentSelectedControl = nullptr;
			ConfigureSelectedControlForm();
		}

		if (e->Data.Forms.RaisedBy->GetForm() == previewselector.get() &&
		    e->Data.Forms.RaisedBy->GetParent()->Name == "FORM_LIST")
		{

			if (currentSelected != nullptr)
			{
				currentSelected->BackgroundColour.a = 0;
			}

			currentSelected = (Label *)e->Data.Forms.RaisedBy;
			currentSelected->BackgroundColour.a = 255;
			displayform.reset(fw().gamecore->GetForm(currentSelected->Name));

			return;
		}
	}
}

void FormPreview::Update(StageCmd *const cmd)
{
	previewselector->Update();
	if (propertyeditor != nullptr)
	{
		propertyeditor->Update();
	}
	if (displayform != nullptr)
	{
		displayform->Update();
	}
	*cmd = this->stageCmd;
	// Reset the command to default
	this->stageCmd = StageCmd();

	glowindex = (glowindex + 4) % 511;
}

void FormPreview::Render()
{
	if (displayform != nullptr)
	{
		displayform->Render();
	}
	previewselector->Render();
	if (propertyeditor != nullptr)
	{
		propertyeditor->Render();
	}

	if (currentSelectedControl != nullptr)
	{
		Vec2<int> border = currentSelectedControl->GetLocationOnScreen();
		if (glowindex < 256)
		{
			fw().renderer->drawRect(border, currentSelectedControl->Size,
			                        OpenApoc::Colour(glowindex, glowindex, glowindex), 3.0f);
		}
		else
		{
			int revglow = 255 - (glowindex - 256);
			fw().renderer->drawRect(border, currentSelectedControl->Size,
			                        OpenApoc::Colour(revglow, revglow, revglow), 3.0f);
		}
	}

	fw().gamecore->MouseCursor->Render();
}

bool FormPreview::IsTransition() { return false; }

void FormPreview::ConfigureSelectedControlForm()
{

	if (currentSelectedControl == nullptr)
	{
	}
	else
	{
	}
}

}; // namespace OpenApoc
