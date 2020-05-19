import subprocess
from scannerpy import Database, DeviceType, Job, BulkJob
from scannerpy.stdlib import NetDescriptor
import numpy as np
import cv2
import struct
import sys
import os
sys.path.append(os.path.dirname(os.path.abspath(__file__)) + '/..')
sys.path.append('/opt/scanner/examples')
sys.path.append('/opt/scanner')
import util

video_path = '/app/foo.mp4' if len(sys.argv) <= 1 else sys.argv[1]
video_name = os.path.splitext(os.path.basename(video_path))[0]

video2_path = '/app/foo2.mp4' if len(sys.argv) <= 2 else sys.argv[2]
video2_name = os.path.splitext(os.path.basename(video_path))[0]

print video_path
print video2_path

with Database() as db:
    #[input_table]
    [input_table, input_table2], _ = db.ingest_videos(
        [('video1', video_path), ('video2', video2_path)], force=True)

    frame = db.ops.FrameInput()
    combined = db.ops.Overlay(frame1=frame, frame2=frame, device=DeviceType.GPU)

    output = db.ops.Output(columns=[combined])

    job = Job(op_args={
        frame: input_table.column('frame'),
        output: input_table.name() + '_output'
    })
    bulk_job = BulkJob(output=output, jobs=[job])

    [output] = db.run(bulk_job, pipeline_instances_per_node=-1, tasks_in_queue_per_pu=10, force=True)
    output.column('frame_output').save_mp4('out')
    #output.column('frame_output2').save_mp4('out2')
    #subprocess.check_call(['ffmpeg', '-y', '-i', 'concat:{}|{}'.format('out1.mp4', 'out2.mp4'), '-c', 'copy', 'out.mp4']);
