/* ===========================================================================
Copyright (C) 2002-2005  3Dlabs Inc. Ltd.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

    Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

    Redistributions in binary form must reproduce the above
    copyright notice, this list of conditions and the following
    disclaimer in the documentation and/or other materials provided
    with the distribution.

    Neither the name of 3Dlabs Inc. Ltd. nor the names of its
    contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
=========================================================================== */

#include "ogl2particle.h"

static GLint arrayWidth, arrayHeight;
static GLfloat *verts = NULL;
static GLfloat *colors = NULL;
static GLfloat *velocities = NULL;
static GLfloat *startTimes = NULL;

/*public*/
void createPoints(GLint w, GLint h)
{
	GLfloat *vptr, *cptr, *velptr, *stptr;
	GLfloat i, j;

	if (verts != NULL) 
		free(verts);

	verts  = malloc(w * h * 3 * sizeof(float));
	colors = malloc(w * h * 3 * sizeof(float));
	velocities = malloc(w * h * 3 * sizeof(float));
	startTimes = malloc(w * h * sizeof(float));

	vptr = verts;
	cptr = colors;
	velptr = velocities;
	stptr  = startTimes;

	for (i = 0.5 / w - 0.5; i < 0.5; i = i + 1.0/w)
		for (j = 0.5 / h - 0.5; j < 0.5; j = j + 1.0/h)
		{
			*vptr       = i;
			*(vptr + 1) = 0.0;
			*(vptr + 2) = j;
			vptr += 3;

			*cptr       = ((float) rand() / RAND_MAX) * 0.5 + 0.5;
			*(cptr + 1) = ((float) rand() / RAND_MAX) * 0.5 + 0.5;
			*(cptr + 2) = ((float) rand() / RAND_MAX) * 0.5 + 0.5;
			cptr += 3;

			*velptr       = (((float) rand() / RAND_MAX)) + 3.0;
			*(velptr + 1) =  ((float) rand() / RAND_MAX) * 10.0;
			*(velptr + 2) = (((float) rand() / RAND_MAX)) + 3.0;
			velptr += 3;

			*stptr = ((float) rand() / RAND_MAX) * 10.0;
			stptr++;
		}

	arrayWidth  = w;
	arrayHeight = h;
}

/*public*/
void drawPoints()
{	
	glPointSize(2.0);

	glVertexPointer(3, GL_FLOAT, 0, verts);
	glColorPointer(3, GL_FLOAT, 0, colors);
	glVertexAttribPointer(VELOCITY_ARRAY,  3, GL_FLOAT, GL_FALSE, 0, velocities);
	glVertexAttribPointer(START_TIME_ARRAY, 1, GL_FLOAT, GL_FALSE, 0, startTimes);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableVertexAttribArray(VELOCITY_ARRAY);
	glEnableVertexAttribArray(START_TIME_ARRAY);

	glDrawArrays(GL_POINTS, 0, arrayWidth * arrayHeight);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableVertexAttribArray(VELOCITY_ARRAY);
	glDisableVertexAttribArray(START_TIME_ARRAY);

}
