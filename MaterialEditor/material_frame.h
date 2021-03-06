/*	Copyright (C) 2011-2016 OldTimes Software

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

	See the GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifndef MATERIAL_FRAME_H
#define	MATERIAL_FRAME_H

typedef enum
{
	MATERIAL_PREVIEW_CUBE,
	MATERIAL_PREVIEW_SPHERE,
	MATERIAL_PREVIEW_PLANE
} MaterialViewportModel;

class MaterialViewportPanel : public BaseViewportPanel
{
public:
	MaterialViewportPanel(wxWindow *wParent);

	virtual void Initialize();
	virtual void Draw();

	bool SetPreviewMaterial(Material_t *NewMaterial);
	void SetModel(MaterialViewportModel PreviewModel);

	Material_t *GetMaterial();
protected:
private:
	DynamicLight_t	*preview_light;
	ClientEntity_t	*preview_entity;
	Material_t		*preview_material;
	model_t			*CubeModel, *SphereModel;

	void ViewEvent(wxCommandEvent &event);

	wxDECLARE_EVENT_TABLE();
};

class CMaterialFrame : public wxFrame
{
public:

	CMaterialFrame(wxWindow* parent, wxWindowID id = wxID_ANY);
	~CMaterialFrame();

	void LoadMaterial(wxString path);
	void UnloadMaterial();
	void ReloadMaterial();

	virtual bool Destroy();

private:

	time_t tCurrentModified, tLastModified;

	wxAuiManager *mManager;
	wxString sCurrentFilePath, sOldFilePath;
	wxStyledTextCtrl *cMaterialScript;

	MaterialViewportPanel *Viewport;

	// Events
	void FileEvent(wxCommandEvent &event);
	void ViewEvent(wxCommandEvent &event);

	wxDECLARE_EVENT_TABLE();
};

#endif // !MATERIAL_FRAME_H

