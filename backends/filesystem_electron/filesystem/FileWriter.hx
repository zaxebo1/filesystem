/*
 * Copyright (c) 2003-2015, GameDuell GmbH
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

package filesystem;

import js.html.ArrayBuffer;

import types.Data;

import js.node.Fs;
import js.node.buffer.Buffer;

class FileWriter
{
    private var fileData : Data;

    private var currentSeekPosition = 0;
    public var seekPosition (get, set) : Int;
    private var path : String;

    public function new( path:String )
    {
        this.path = path;
    }

    public function get_seekPosition () : Int
    {
        return currentSeekPosition;
    }

    public function set_seekPosition (val : Int) : Int
    {
        currentSeekPosition = val;
        return currentSeekPosition;
    }

    public function writeFromData(data : Data)
    {
        var buffer = new Buffer(data.allocedLength);
        var uint8Array = data.uint8Array;
        var len = uint8Array.length;
        for ( i in 0...len )
        {
            buffer.writeInt8( uint8Array[i], i );
        }

        Fs.appendFileSync(path, buffer);
    }

    public function close()
    {

    }
}